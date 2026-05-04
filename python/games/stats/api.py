from fastapi import APIRouter
from fastapi import HTTPException
from pydantic import BaseModel
from typing import List, Literal
from games.common import stats_manager

router = APIRouter()


class BunkerStatsRequest(BaseModel):
    survivors: List[str]   # agent_type names
    exiled: List[str]      # agent_type names


class MafiaStatsRequest(BaseModel):
    winner: Literal["mafia", "citizens"]
    mafia_agents: List[str]
    citizen_agents: List[str]


class TicTacToeStatsRequest(BaseModel):
    agent_x: str
    agent_o: str
    winner: Literal["X", "O", "draw"]


def _validate_agent_list(names: List[str], field: str) -> None:
    if not names:
        raise HTTPException(status_code=400, detail=f"{field} must not be empty")
    for name in names:
        if not name or not isinstance(name, str) or not name.strip():
            raise HTTPException(status_code=400, detail=f"{field} contains an empty agent name")


@router.post("/record/bunker")
async def record_bunker(req: BunkerStatsRequest):
    _validate_agent_list(req.survivors + req.exiled, "bunker agents")
    stats_manager.record_bunker(req.survivors, req.exiled)
    return {"ok": True}


@router.post("/record/mafia")
async def record_mafia(req: MafiaStatsRequest):
    _validate_agent_list(req.mafia_agents, "mafia_agents")
    _validate_agent_list(req.citizen_agents, "citizen_agents")
    stats_manager.record_mafia(req.winner, req.mafia_agents, req.citizen_agents)
    return {"ok": True}


@router.post("/record/tictactoe")
async def record_tictactoe(req: TicTacToeStatsRequest):
    _validate_agent_list([req.agent_x, req.agent_o], "tictactoe agents")
    stats_manager.record_tictactoe(req.agent_x, req.agent_o, req.winner)
    return {"ok": True}


@router.get("/report")
async def get_report():
    return stats_manager.get_report()
