
from pydantic import BaseModel
from typing import List, Optional

class ChatMessage(BaseModel):
    player_id: int
    player_name: str
    player_role: str
    text: str
    timestamp: str = ""
    is_night: bool = False
    is_public: bool = True

class PlayerState(BaseModel):
    id: int
    name: str
    role: str = "citizen"
    is_alive: bool = True
    is_protected: bool = False
    votes_against: int = 0

class GameState(BaseModel):
    phase: str  # NIGHT_MAFIA, DAY_DISCUSSION, etc.
    day: int
    players: List[PlayerState]
    chat_history: List[ChatMessage]
    known_roles: Optional[dict] = None 

class AgentAction(BaseModel):
    action_type: str # VOTE_KILL, MAFIA_KILL, SHERIFF_CHECK, etc.
    target_id: int = -1
    text_message: str = ""
