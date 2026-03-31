from fastapi import APIRouter
from pydantic import BaseModel
from typing import List
import random

from .models import BunkerAction, BunkerState, PlayerState, ChatMessage
from .agents.llm_gemma3_agent import BunkerLLMAgent

router = APIRouter()


class BunkerAgentRequest(BaseModel):
    agent_name: str
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
    players = [PlayerState(**p) for p in req.players]
    chat = [ChatMessage(**m) for m in req.chat_history]
    my_id = int(req.my_character.get("player_id", -1))
    return BunkerState(
        phase=phase_to_str(req.phase),
        players=players,
        chat_history=chat,
        my_player_id=my_id,
        known_info=req.known_info or [],
    )


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
    name_upper = req.agent_name.upper()
    state = build_state(req)

    if "LLM" in name_upper:
        action = BunkerLLMAgent(req.agent_name).decide(state)
        # safety-check target
        if action.action_type == "VOTE_EXILE":
            alive_ids = {p.player_id for p in state.players if p.is_alive and p.player_id != state.my_player_id}
            if action.target_id not in alive_ids:
                return fallback_action(req)
        return action

    return fallback_action(req)


@router.post("/agent_chat", response_model=ChatResponse)
async def bunker_agent_chat(req: BunkerAgentRequest):
    name_upper = req.agent_name.upper()
    state = build_state(req)

    if "LLM" in name_upper:
        action = BunkerLLMAgent(req.agent_name).decide(state)
        message = (action.text_message or "").strip()
        if message:
            return ChatResponse(message=message[:400])
        return ChatResponse(message="Предлагаю выбирать тех, кто полезнее для выживания.")

    # simple fallback phrases
    phase = phase_to_str(req.phase)
    if phase == "DISCUSSION":
        return ChatResponse(message="Нам нужно оставить в бункере самых полезных игроков.")
    return ChatResponse(message="")

