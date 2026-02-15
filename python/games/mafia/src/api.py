
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
    # Convert request to internal GameState
    # This mapping is crucial. For now, let's assume a simplified mapping.
    
    # Map C++ Phase enum to string
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

    # Reconstruct simplified GameState from request
    # Since specific game day isn't passed, we can infer or pass 1
    # For stateless agents, exact day might not be critical yet
    
    # We need to map raw dicts back to Pydantic models if strict validation is needed
    # But AgentFactory just needs a structure with .phase, .players, etc.
    
    # Let's create a dynamic object or reuse GameState model
    # GameState model expects List[PlayerState], List[ChatMessage]
    # We can do a quick conversion or just pass the raw dict if agents are flexible.
    # But our agents expect .phase attribute.
    
    from .models import GameState, PlayerState, ChatMessage
    
    players = [PlayerState(**p) for p in request.players]
    chat = [ChatMessage(**c) for c in request.chat_history]
    
    game_state = GameState(
        phase=phase_str,
        day=1, # TODO: pass day from C++
        players=players,
        chat_history=chat,
        known_roles={} 
    )

    action = AgentFactory.get_action_for_player(game_state, request.role, request.agent_name)
    return action

class ChatResponse(BaseModel):
    message: str

@router.post("/agent_chat", response_model=ChatResponse)
async def get_agent_chat(request: AgentRequest):
    # Same logic as above
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
    
    from .models import GameState, PlayerState, ChatMessage
    
    players = [PlayerState(**p) for p in request.players]
    chat = [ChatMessage(**c) for c in request.chat_history]
    
    game_state = GameState(
        phase=phase_str,
        day=1,
        players=players,
        chat_history=chat
    )
    
    # We need a method in AgentFactory or Agent to get chat message
    # Currently AgentFactory only has get_action_for_player
    # Let's use get_agent directly for chat
    
    agent = AgentFactory.get_agent("RANDOM", request.agent_name)
    
    # We need a method in BaseAgent to get chat.
    # Currently decide_action returns AgentAction witch can be CHAT_MESSAGE
    
    action = agent.decide_action(game_state, request.role)
    
    # If the action is CHAT_MESSAGE, return text. Else return empty or generic.
    if action.action_type == "CHAT_MESSAGE":
        return ChatResponse(message=action.text_message)
    else:
        # If agent decided to PASS or VOTE during chat phase (which shouldn't happen if logic is correct),
        # fallback to generic message
        return ChatResponse(message="...")
