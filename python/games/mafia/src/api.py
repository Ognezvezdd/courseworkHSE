from fastapi import APIRouter
from pydantic import BaseModel
from typing import List, Optional, Dict
from .agent_factory import AgentFactory
from .models import AgentAction, GameState, VotingRecord

router = APIRouter()


class AgentRequest(BaseModel):
    agent_name: str
    phase: int                          # C++ Phase enum
    role: str                           # Роль агента
    players: List[dict]                 # Анонимизированные игроки
    chat_history: List[dict]
    known_info: List[str]               # Личная информация агента (результаты проверок)
    # Расширенный контекст:
    my_id: Optional[int] = -1
    voting_history: Optional[List[dict]] = []       # История голосований по дням
    eliminated_players: Optional[List[str]] = []    # Кто и когда выбыл
    mafia_team_ids: Optional[List[int]] = []        # Союзники (только для мафии)
    known_roles: Optional[Dict[str, str]] = {}      # Раскрытые роли (шерифом/доном)


PHASE_MAP = {
    0: "NIGHT_MAFIA",
    1: "NIGHT_DON",
    2: "NIGHT_SHERIFF",
    3: "NIGHT_DOCTOR",
    4: "NIGHT_RESULTS",
    5: "DAY_DISCUSSION",
    6: "DAY_VOTING",
    7: "DAY_RESULTS",
    8: "GAME_END",
}


@router.post("/agent_action", response_model=AgentAction)
async def get_agent_action(request: AgentRequest):
    game_state = prepare_game_state(request)

    agent_type = "RANDOM"
    name_upper = request.agent_name.upper()
    if "LLM" in name_upper:
        agent_type = "LLM"
    elif "RL" in name_upper or "QLEARNING" in name_upper:
        agent_type = "RL"

    action = AgentFactory.get_action_for_player(
        game_state, request.role, request.agent_name, agent_type
    )
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

    if action.action_type == "CHAT_MESSAGE" and action.text_message.strip():
        return ChatResponse(message=action.text_message)
    return ChatResponse(message="")


def prepare_game_state(request: AgentRequest) -> GameState:
    from .models import PlayerState, ChatMessage

    phase_str = PHASE_MAP.get(request.phase, "UNKNOWN")

    # Строим анонимный маппинг: настоящее_имя → Player_N
    # Сортируем по id для стабильного порядка
    raw_players = sorted(request.players, key=lambda p: p.get("id", 0))
    name_to_alias: Dict[str, str] = {}
    for i, p in enumerate(raw_players):
        original_name = p.get("name", f"Player_{i+1}")
        name_to_alias[original_name] = f"Player_{i+1}"

    # Создаём анонимизированных игроков (роли НЕ передаём в name)
    players = []
    for p in raw_players:
        alias = name_to_alias.get(p.get("name", ""), p.get("name", ""))
        players.append(PlayerState(
            id=p.get("id", -1),
            name=alias,                      # Анонимизированное имя
            role=p.get("role", "CITIZEN"),   # Роль — только для логики агента
            is_alive=p.get("is_alive", True),
            is_protected=p.get("is_protected", False),
            votes_against=p.get("votes_against", 0),
        ))

    # Анонимизируем историю чата
    chat = []
    for c in request.chat_history:
        raw_name = c.get("player_name", "")
        alias = name_to_alias.get(raw_name, raw_name)
        # Системные сообщения (player_id == -1) пропускаем — не показываем агентам
        pid = c.get("player_id", -1)
        if pid == -1:
            continue
        chat.append(ChatMessage(
            player_id=pid,
            player_name=alias,
            player_role="",           # Роль из чата НЕ передаём (утечка!)
            text=c.get("text", ""),
            timestamp=c.get("timestamp", ""),
            is_night=c.get("is_night", False),
            is_public=c.get("is_public", True),
        ))

    # Находим my_id
    my_id = request.my_id if request.my_id and request.my_id != -1 else -1
    if my_id == -1:
        for p in raw_players:
            if p.get("name") == request.agent_name:
                my_id = p.get("id", -1)
                break

    # История голосований
    voting_history = []
    for record in (request.voting_history or []):
        # Анонимизируем имена в голосованиях
        anon_votes = {}
        for raw_name, votes in (record.get("votes") or {}).items():
            anon_votes[name_to_alias.get(raw_name, raw_name)] = votes
        exiled_raw = record.get("exiled")
        exiled_anon = name_to_alias.get(exiled_raw, exiled_raw) if exiled_raw else None
        voting_history.append(VotingRecord(
            day=record.get("day", 0),
            votes=anon_votes,
            exiled=exiled_anon,
        ))

    # Список выбывших (уже должен содержать анонимные имена)
    eliminated = request.eliminated_players or []

    # Раскрытые роли (ключи — ID игроков)
    known_roles = {}
    for pid_str, role in (request.known_roles or {}).items():
        try:
            pid = int(pid_str)
            known_roles[pid] = role
        except ValueError:
            continue

    return GameState(
        phase=phase_str,
        day=max(1, len(voting_history) + 1),  # Выводим день из истории
        players=players,
        chat_history=chat,
        my_id=my_id,
        my_role=request.role.upper(),
        known_roles=known_roles,
        voting_history=voting_history,
        eliminated_players=eliminated,
        mafia_team_ids=request.mafia_team_ids or [],
    )
