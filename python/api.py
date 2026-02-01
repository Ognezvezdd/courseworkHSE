
import sys
import os
from typing import List, Optional, Dict, Any
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel

# Add current directory to sys.path to ensure modules can be imported
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from agents.heuristic_agent import HeuristicAgent
from agents.qlearning_agent import QLearningAgent
from agents.random_agent import RandomAgent
from agents.base_agent import BaseAgent
from game.engine import run_game

app = FastAPI(
    title="AI Agents Platform API",
    description="API for running AI agents games and training",
    version="1.0.0"
)

AGENT_CLASSES = {
    "random": RandomAgent,
    "heuristic": HeuristicAgent,
    "qlearning": QLearningAgent,
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

class TrainResult(BaseModel):
    success: bool
    stats: Dict[str, Any]

@app.get("/")
async def root():
    return {"message": "AI Agents Platform API is running"}

@app.get("/agents")
async def get_agents():
    """Return list of available agent types."""
    return {"agents": list(AGENT_CLASSES.keys())}

def create_agent(agent_type: str, name_suffix: str = "") -> BaseAgent:
    if agent_type not in AGENT_CLASSES:
        raise HTTPException(status_code=400, detail=f"Unknown agent type: {agent_type}")
    
    agent_cls = AGENT_CLASSES[agent_type]
    # Some agents might take a name argument, others might not, 
    # but based on BaseAgent they usually do or have default.
    # Looking at the codebase, they can take name.
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
        
    return GameResult(
        winner=winner,
        steps=len(slides),
        slides=slides
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
