import json
import time

import requests


API_URL = "http://localhost:8000/mafia"

request_data = {
    "agent_name": "Agent_LLM",
    "phase": 6,
    "role": "citizen",
    "players": [
        {"id": 1, "name": "Agent_LLM", "role": "citizen", "is_alive": True, "is_protected": False, "votes_against": 0},
        {"id": 2, "name": "Bob", "role": "unknown", "is_alive": True, "is_protected": False, "votes_against": 0},
        {"id": 3, "name": "Charlie", "role": "unknown", "is_alive": False, "is_protected": False, "votes_against": 0},
    ],
    "chat_history": [
        {"player_id": 2, "player_name": "Bob", "player_role": "unknown", "text": "I think Charlie was mafia", "timestamp": "12345", "is_night": False, "is_public": True}
    ],
    "known_info": ["You are citizen."],
}


def assert_action_response(payload: dict) -> None:
    assert "action_type" in payload
    assert payload["action_type"] in {
        "VOTE_KILL",
        "MAFIA_KILL",
        "SHERIFF_CHECK",
        "DOCTOR_HEAL",
        "DON_CHECK",
        "CHAT_MESSAGE",
        "PASS",
    }
    assert isinstance(payload.get("target_id", -1), int)
    assert isinstance(payload.get("text_message", ""), str)


def assert_chat_response(payload: dict) -> None:
    assert "message" in payload
    assert isinstance(payload["message"], str)


if __name__ == "__main__":
    start_time = time.time()
    response = requests.post(f"{API_URL}/agent_action", json=request_data, timeout=60)
    assert response.status_code == 200, response.text
    action_payload = response.json()
    assert_action_response(action_payload)

    chat_response = requests.post(f"{API_URL}/agent_chat", json=request_data, timeout=60)
    assert chat_response.status_code == 200, chat_response.text
    chat_payload = chat_response.json()
    assert_chat_response(chat_payload)

    print(f"Mafia LLM API check passed in {time.time() - start_time:.2f}s")
    print(json.dumps({"action": action_payload, "chat": chat_payload}, indent=2, ensure_ascii=False))
