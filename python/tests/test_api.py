from fastapi.testclient import TestClient
import sys
import os

# Add the project root to sys.path to ensure we can import 'api' and other modules
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from api import app

client = TestClient(app)

def test_read_main():
    response = client.get("/")
    assert response.status_code == 200
    assert response.json() == {"message": "AI Agents Platform API is running"}

def test_get_agents():
    response = client.get("/agents")
    assert response.status_code == 200
    data = response.json()
    assert "agents" in data
    assert "random" in data["agents"]
    assert "heuristic" in data["agents"]

def test_play_game_valid():
    payload = {
        "agent_x": "random",
        "agent_o": "heuristic",
        "seed": 42
    }
    response = client.post("/game/play", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert "winner" in data
    assert "steps" in data
    assert "slides" in data
    assert len(data["slides"]) > 0
    assert data["winner"] in ["X", "O", "draw"]

def test_play_game_invalid_agent():
    payload = {
        "agent_x": "unknown_agent",
        "agent_o": "random"
    }
    response = client.post("/game/play", json=payload)
    assert response.status_code == 400

def test_train_agent():
    payload = {
        "agent_type": "qlearning",
        "episodes": 10,
        "seed": 42
    }
    response = client.post("/train", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "stats" in data

def test_train_agent_invalid_type():
    payload = {
        "agent_type": "random",
        "episodes": 10
    }
    response = client.post("/train", json=payload)
    assert response.status_code == 400
