import uvicorn
from fastapi import FastAPI
from games.tictactoe.src.api import app as tictactoe_app
from games.mafia.src.api import router as mafia_router

# Use tictactoe app as base app for backward compatibility 
app = tictactoe_app

# Include mafia router
# Note: tictactoe app endpoints (like /game/play) are already mounted/included in it.
app.include_router(mafia_router, prefix="/mafia", tags=["mafia"])

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
