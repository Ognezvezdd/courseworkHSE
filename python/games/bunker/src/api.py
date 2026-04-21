from fastapi import APIRouter
from pydantic import BaseModel
from typing import List
import random

from .models import BunkerAction, BunkerState, PlayerState, ChatMessage
from .agents.llm_gemma3_agent import BunkerLLMAgent

router = APIRouter()


class BunkerAgentRequest(BaseModel):
    agent_name: str
    agent_type: str = "llm_gemma3" # Тип модели и личности
    phase: int
    players: List[dict]
    chat_history: List[dict]
    my_character: dict
    known_info: List[str]


class ChatResponse(BaseModel):
    message: str


def phase_to_str(phase: int) -> str:
    phase_map = {
        0: "WAITING",
        1: "CHARACTER_CREATION",
        2: "DISCUSSION",
        3: "VOTING",
        4: "EXILE",
        5: "GAME_OVER",
    }
    return phase_map.get(phase, "UNKNOWN")


def build_state(req: BunkerAgentRequest) -> BunkerState:
    players = []
    for p in req.players:
        # Убеждаемся, что численные параметры передаются
        players.append(PlayerState(
            player_id=p.get("player_id", -1),
            player_name=p.get("player_name", "Unknown"),
            profession=p.get("profession", "UNKNOWN"),
            age=p.get("age", 30),
            health=p.get("health", "HEALTHY"),
            survival_score=p.get("survival_score", 50),
            utility_score=p.get("utility_score", 50),
            skills=p.get("skills", []),
            personality=p.get("personality", "CALM"),
            is_alive=p.get("is_alive", True),
            is_exiled=p.get("is_exiled", False)
        ))
        
    chat = [ChatMessage(**m) for m in req.chat_history]
    my_id = int(req.my_character.get("player_id", -1))
    return BunkerState(
        phase=phase_to_str(req.phase),
        players=players,
        chat_history=chat,
        my_player_id=my_id,
        known_info=req.known_info or [],
    )


def get_personality_and_model(agent_type: str):
    name_upper = agent_type.upper()
    personality = "default"
    model = "gemma3"
    
    if "RATIONAL" in name_upper: personality = "rational"
    elif "AGGRESSIVE" in name_upper: personality = "aggressive"
    elif "COOPERATIVE" in name_upper: personality = "cooperative"
    elif "EMOTIONAL" in name_upper: personality = "emotional"
    elif "SURVIVOR" in name_upper: personality = "survivor"
    elif "SKEPTIC" in name_upper: personality = "skeptic"
    
    # Мапим модели на короткие имена из MODEL_MAP
    if "LLAMA3.2_1B" in name_upper: model = "llama3.2_1b"
    elif "LLAMA3.2_3B" in name_upper: model = "llama3.2_3b"
    elif "PHI4" in name_upper: model = "phi4_mini"
    elif "PHI3" in name_upper: model = "phi3_mini"
    elif "QWEN" in name_upper: model = "qwen2.5_1.5b"
    elif "GEMMA" in name_upper: model = "gemma3"
    
    return personality, model


def fallback_action(req: BunkerAgentRequest) -> BunkerAction:
    phase = phase_to_str(req.phase)
    if phase != "VOTING":
        return BunkerAction(action_type="PASS")
    my_id = int(req.my_character.get("player_id", -1))
    alive = [p for p in req.players if p.get("is_alive", False) and p.get("player_id") != my_id]
    if not alive:
        return BunkerAction(action_type="PASS")
    target = random.choice(alive)["player_id"]
    return BunkerAction(action_type="VOTE_EXILE", target_id=int(target))


@router.post("/agent_action", response_model=BunkerAction)
async def bunker_agent_action(req: BunkerAgentRequest):
    state = build_state(req)
    # Проверяем agent_type, так как agent_name теперь анонимизирован (Player_N)
    if "LLM" in req.agent_type.upper() or "BUNKER" in req.agent_type.upper():
        pers, model = get_personality_and_model(req.agent_type)
        action = BunkerLLMAgent(req.agent_name, model=model, personality=pers).decide(state)
        # safety-check target
        if action.action_type == "VOTE_EXILE":
            alive_ids = {p.player_id for p in state.players if p.is_alive and p.player_id != state.my_player_id}
            if action.target_id not in alive_ids:
                return fallback_action(req)
        return action

    return fallback_action(req)


@router.post("/agent_chat", response_model=ChatResponse)
async def bunker_agent_chat(req: BunkerAgentRequest):
    state = build_state(req)
    if "LLM" in req.agent_type.upper() or "BUNKER" in req.agent_type.upper():
        pers, model = get_personality_and_model(req.agent_type)
        action = BunkerLLMAgent(req.agent_name, model=model, personality=pers).decide(state)
        message = (action.text_message or "").strip()
        if message:
            return ChatResponse(message=message[:400])
        return ChatResponse(message="Давайте обсудим, кто из нас действительно полезен для выживания в долгосрочной перспективе.")

    # simple fallback phrases
    phase = phase_to_str(req.phase)
    if phase == "DISCUSSION":
        return ChatResponse(message="Нам нужно оставить в бункере самых полезных игроков.")
    return ChatResponse(message="")

