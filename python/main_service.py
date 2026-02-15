import uvicorn
from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from games.tictactoe.src.api import app as tictactoe_app
# Placeholder for mafia router when it's ready, for now we will just mount tictactoe
# from games.mafia.src.api import router as mafia_router 

app = FastAPI()

# Mount sub-applications
app.mount("/game", tictactoe_app) # Keep existing tictactoe endpoints working under /game prefix or just include router

# Or better, include routers if they are APIRouters, but tictactoe api.py creates a full app
# intended to run standalone. Let's merge them.

# Actually, the original tictactoe api was running at root. 
# We should keep backward compatibility for now.
# Let's import the routers/endpoints from tictactoe.

# To keep it simple and robust:
# We will use this file as the main entry point. 
# We need to adapt tictactoe/src/api.py to export a router instead of an app, 
# OR just mount it.

app = tictactoe_app # For now, tictactoe IS the main app. 
# We will extend it later with mafia endpoints.

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
