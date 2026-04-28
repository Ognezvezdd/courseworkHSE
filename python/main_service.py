import uvicorn
from fastapi import FastAPI
from games.tictactoe.src.api import app as tictactoe_app
from games.mafia.src.api import router as mafia_router
from games.bunker.src.api import router as bunker_router
from games.stats.api import router as stats_router

# Use tictactoe app as base app for backward compatibility
app = tictactoe_app

# Include game routers
app.include_router(mafia_router, prefix="/mafia", tags=["mafia"])
app.include_router(bunker_router, prefix="/bunker", tags=["bunker"])
app.include_router(stats_router, prefix="/stats", tags=["stats"])

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
