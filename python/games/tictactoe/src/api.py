
import sys
import os
from typing import List, Optional, Dict, Any
import time
import uuid
import matplotlib
matplotlib.use('Agg')  # Set backend for headless environment
from fastapi import FastAPI, HTTPException
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

# Add current directory to sys.path to ensure modules can be imported
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from agents.heuristic_agent import HeuristicAgent
from agents.qlearning_agent import QLearningAgent
from agents.random_agent import RandomAgent
from agents.base_agent import BaseAgent
from agents.llm_gemma3_agent import LLMAgent
from game.engine import run_game
from visualization.renderer import GameRenderer

# Ensure output directory exists
OUTPUT_DIR = "output"
os.makedirs(OUTPUT_DIR, exist_ok=True)

app = FastAPI(
    title="AI Agents Platform API",
    description="API for running AI agents games and training",
    version="1.0.0"
)

# Mount static files for visualizations
app.mount("/static", StaticFiles(directory=OUTPUT_DIR), name="static")

renderer = GameRenderer()

AGENT_CLASSES = {
    "random": RandomAgent,
    "heuristic": HeuristicAgent,
    "qlearning": QLearningAgent,
    "llm": LLMAgent,
}

MODEL_MAP = {
    "gemma3": "gemma3",
    "llama3.2_1b": "llama3.2:1b",
    "llama3.2_3b": "llama3.2:3b",
    "phi3_mini": "phi3:mini",
    "phi4_mini": "phi4-mini",
    "qwen2.5_1.5b": "qwen2.5:1.5b",
}

class GameRequest(BaseModel):
    agent_x: str
    agent_o: str
    seed: Optional[int] = None

class TrainRequest(BaseModel):
    agent_type: str
    episodes: int
    seed: Optional[int] = None
    opponent_type: str = "random"

class GameResult(BaseModel):
    winner: str
    steps: int
    slides: List[Dict[str, Any]]
    image_url: Optional[str] = None
    image_filename: Optional[str] = None

class TrainResult(BaseModel):
    success: bool
    stats: Dict[str, Any]

@app.get("/")
async def root():
    return {"message": "AI Agents Platform API is running"}

@app.get("/agents")
async def get_agents():
    """Return list of available agent types."""
    agents = ["random", "heuristic", "qlearning"]
    # Добавляем LLM-модели для бенчмарка
    for model_key in MODEL_MAP.keys():
        agents.append(f"llm_{model_key}")
    return {"agents": agents}

def create_agent(agent_type: str, name_suffix: str = "") -> BaseAgent:
    # Обработка специфических LLM-моделей (llm_gemma3, llm_llama3.2_1b и т.д.)
    if agent_type.startswith("llm_"):
        model_key = agent_type[4:]
        model_name = MODEL_MAP.get(model_key, model_key.replace("_", ":"))
        return LLMAgent(name=f"Agent_{name_suffix} ({agent_type})", model=model_name)

    if agent_type not in AGENT_CLASSES:
        raise HTTPException(status_code=400, detail=f"Unknown agent type: {agent_type}")
    
    agent_cls = AGENT_CLASSES[agent_type]
    return agent_cls(name=f"Agent_{name_suffix} ({agent_type})")

@app.post("/game/play", response_model=GameResult)
async def play_game(request: GameRequest):
    """Run a game between two agents."""
    agent_x = create_agent(request.agent_x, "X")
    agent_o = create_agent(request.agent_o, "O")
    
    slides = run_game(agent_x, agent_o, seed=request.seed)
    
    winner = slides[-1].get("winner")
    if not winner:
        winner = "draw"
        
    # Generate summary image
    img_filename = f"game_{uuid.uuid4().hex[:8]}.png"
    img_path = os.path.join(OUTPUT_DIR, img_filename)
    
    try:
        renderer.render_summary(slides, filepath=img_path)
        image_url = f"/static/{img_filename}"
    except Exception as e:
        print(f"Error rendering image: {e}")
        image_url = None
        
    return GameResult(
        winner=winner,
        steps=len(slides),
        slides=slides,
        image_url=image_url,
        image_filename=img_filename
    )

@app.post("/train", response_model=TrainResult)
async def train_agent(request: TrainRequest):
    """Train a Q-Learning agent."""
    if request.agent_type != "qlearning":
        raise HTTPException(status_code=400, detail="Only qlearning agent can be trained")
        
    agent = QLearningAgent(name="Trainee")
    opponent = create_agent(request.opponent_type, "Opponent")
    
    stats = agent.train(opponent, episodes=request.episodes, seed=request.seed)
    
    # In a real scenario, we might want to save the q-table here
    # For now, we just return the stats
    
    return TrainResult(
        success=True,
        stats=stats
    )

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
