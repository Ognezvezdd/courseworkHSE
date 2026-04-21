
from pydantic import BaseModel
from typing import List, Optional, Dict


class ChatMessage(BaseModel):
    player_id: int
    player_name: str        # Анонимизированное имя (Player_1, Player_2, ...)
    player_role: str        # Роль — НЕ передается публично (только серверу для логов)
    text: str
    timestamp: str = ""
    is_night: bool = False
    is_public: bool = True


class PlayerState(BaseModel):
    id: int
    name: str               # Анонимизированное имя (Player_1, ...)
    role: str = "CITIZEN"   # Роль — видна только самому агенту
    is_alive: bool = True
    is_protected: bool = False
    votes_against: int = 0


class VotingRecord(BaseModel):
    """История одного раунда голосования"""
    day: int
    votes: Dict[str, int] = {}   # player_name -> votes_received
    exiled: Optional[str] = None  # кого изгнали (или None)


class GameState(BaseModel):
    phase: str                          # NIGHT_MAFIA, DAY_DISCUSSION, etc.
    day: int
    players: List[PlayerState]          # Все игроки (имена анонимизированы)
    chat_history: List[ChatMessage]     # Публичные сообщения
    my_id: int = -1                     # ID этого агента
    my_role: str = "CITIZEN"            # Роль этого агента
    known_roles: Optional[Dict[int, str]] = None  # Раскрытые роли (через шерифа/дона)
    voting_history: List[VotingRecord] = []        # История голосований по дням
    eliminated_players: List[str] = []             # Кто выбыл и когда
    mafia_team_ids: List[int] = []                 # Только для мафии: ID союзников


class AgentAction(BaseModel):
    action_type: str  # VOTE_KILL, MAFIA_KILL, SHERIFF_CHECK, etc.
    target_id: int = -1
    text_message: str = ""
