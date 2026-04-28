from fastapi import APIRouter
from pydantic import BaseModel
from typing import List, Optional
from games.common import stats_manager

router = APIRouter()


class BunkerStatsRequest(BaseModel):
    survivors: List[str]   # agent_type names
    exiled: List[str]      # agent_type names


class MafiaStatsRequest(BaseModel):
    winner: str            # "mafia" | "citizens"
    mafia_agents: List[str]
    citizen_agents: List[str]


class TicTacToeStatsRequest(BaseModel):
    agent_x: str
    agent_o: str
    winner: str            # "X" | "O" | "draw"


@router.post("/record/bunker")
async def record_bunker(req: BunkerStatsRequest):
    stats_manager.record_bunker(req.survivors, req.exiled)
    return {"ok": True}


@router.post("/record/mafia")
async def record_mafia(req: MafiaStatsRequest):
    stats_manager.record_mafia(req.winner, req.mafia_agents, req.citizen_agents)
    return {"ok": True}


@router.post("/record/tictactoe")
async def record_tictactoe(req: TicTacToeStatsRequest):
    stats_manager.record_tictactoe(req.agent_x, req.agent_o, req.winner)
    return {"ok": True}


@router.get("/report")
async def get_report():
    return stats_manager.get_report()
