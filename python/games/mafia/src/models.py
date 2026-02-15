
from pydantic import BaseModel
from typing import List, Optional

class ChatMessage(BaseModel):
    player_id: int
    player_name: str
    player_role: str
    text: str
    is_public: bool

class PlayerState(BaseModel):
    id: int
    name: str
    is_alive: bool
    votes_against: int = 0

class GameState(BaseModel):
    phase: str  # NIGHT_MAFIA, DAY_DISCUSSION, etc.
    day: int
    players: List[PlayerState]
    chat_history: List[ChatMessage]
    # Для мафии, дона, шерифа, доктора - их специфичная инфа
    known_roles: Optional[dict] = None 

class AgentAction(BaseModel):
    action_type: str # VOTE_KILL, MAFIA_KILL, SHERIFF_CHECK, etc.
    target_id: int = -1
    text_message: str = ""
