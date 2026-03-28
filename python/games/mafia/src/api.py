
from fastapi import APIRouter
from pydantic import BaseModel
from typing import List, Optional
from .agent_factory import AgentFactory
from .models import AgentAction, GameState

router = APIRouter()

class AgentRequest(BaseModel):
    agent_name: str
    phase: int # C++ Phase enum
    role: str
    players: List[dict] # Simplified for now
    chat_history: List[dict]
    known_info: List[str]

@router.post("/agent_action", response_model=AgentAction)
async def get_agent_action(request: AgentRequest):
    game_state = prepare_game_state(request)
    
    agent_type = "RANDOM"
    name_upper = request.agent_name.upper()
    if "LLM" in name_upper:
        agent_type = "LLM"
    elif "RL" in name_upper or "QLEARNING" in name_upper:
        agent_type = "RL"
        
    action = AgentFactory.get_action_for_player(game_state, request.role, request.agent_name, agent_type)
    return action

class ChatResponse(BaseModel):
    message: str

@router.post("/agent_chat", response_model=ChatResponse)
async def get_agent_chat(request: AgentRequest):
    game_state = prepare_game_state(request)
    
    agent_type = "RANDOM"
    name_upper = request.agent_name.upper()
    if "LLM" in name_upper:
        agent_type = "LLM"
    elif "RL" in name_upper or "QLEARNING" in name_upper:
        agent_type = "RL"
        
    agent = AgentFactory.get_agent(agent_type, request.agent_name)
    action = agent.decide_action(game_state, request.role)
    
    if action.action_type == "CHAT_MESSAGE":
        return ChatResponse(message=action.text_message)
    return ChatResponse(message="...")

def prepare_game_state(request: AgentRequest) -> GameState:
    from .models import GameState, PlayerState, ChatMessage
    
    phase_map = {
        0: "NIGHT_MAFIA",
        1: "NIGHT_DON",
        2: "NIGHT_SHERIFF", 
        3: "NIGHT_DOCTOR", 
        4: "NIGHT_RESULTS",
        5: "DAY_DISCUSSION",
        6: "DAY_VOTING",
        7: "DAY_RESULTS",
        8: "GAME_END"
    }
    
    phase_str = phase_map.get(request.phase, "UNKNOWN")
    players = [PlayerState(**p) for p in request.players]
    chat = [ChatMessage(**c) for c in request.chat_history]
    
    # Пытаемся найти ID текущего агента
    my_id = -1
    for p in players:
        if p.name == request.agent_name:
            my_id = p.id
            break

    return GameState(
        phase=phase_str,
        day=1, # Можно инферить из логов если нужно
        players=players,
        chat_history=chat,
        my_id=my_id,
        known_roles={} 
    )
