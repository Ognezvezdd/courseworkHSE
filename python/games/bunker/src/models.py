from pydantic import BaseModel
from typing import List


class ChatMessage(BaseModel):
    player_id: int
    player_name: str
    text: str
    timestamp: str = ""
    is_public: bool = True


class PlayerState(BaseModel):
    player_id: int
    player_name: str
    profession: str = "UNKNOWN"
    age: int = 30
    health: str = "HEALTHY"
    skills: List[str] = []
    personality: str = "CALM"
    is_alive: bool = True
    is_exiled: bool = False
    survival_score: int = 50
    utility_score: int = 50


class BunkerState(BaseModel):
    phase: str
    players: List[PlayerState]
    chat_history: List[ChatMessage]
    my_player_id: int = -1
    known_info: List[str] = []


class BunkerAction(BaseModel):
    action_type: str  # VOTE_EXILE or PASS
    target_id: int = -1
    text_message: str = ""
    used_fallback: bool = False
    fallback_reason: str = ""
