import requests
import json
import time

API_URL = "http://localhost:8000/mafia"

# Моковая фаза: DAY_VOTING 
request_data = {
    "agent_name": "Agent_LLM",
    "phase": 6, # 6 = DAY_VOTING in C++ enum
    "role": "citizen", 
    "players": [
        {"id": 1, "name": "Agent_LLM", "role": "citizen", "is_alive": True, "is_protected": False, "votes_against": 0},
        {"id": 2, "name": "Bob", "role": "unknown", "is_alive": True, "is_protected": False, "votes_against": 0},
        {"id": 3, "name": "Charlie", "role": "unknown", "is_alive": False, "is_protected": False, "votes_against": 0}
    ],
    "chat_history": [
        {"player_id": 2, "player_name": "Bob", "player_role": "unknown", "text": "I think Charlie was mafia", "timestamp": "12345", "is_night": False, "is_public": True}
    ],
    "known_info": ["You are citizen."]
}

print("Отправка запроса к API Мафии для LLM (agent_action)...\n")
try:
    start_time = time.time()
    response = requests.post(
        f"{API_URL}/agent_action",
        json=request_data,
        timeout=60
    )
    print(f"Ответ API (заняло {time.time() - start_time:.2f} сек):")
    print(f"Status Code: {response.status_code}")
    if response.status_code == 200:
        print(json.dumps(response.json(), indent=2))
    else:
        print(response.text)
        
    print("\nОтправка запроса на получение сообщения (agent_chat)...")
    chat_response = requests.post(
        f"{API_URL}/agent_chat",
        json=request_data,
        timeout=60
    )
    print(f"Ответ Чата:")
    if chat_response.status_code == 200:
        print(json.dumps(chat_response.json(), indent=2, ensure_ascii=False))
    else:
        print(chat_response.text)

except Exception as e:
    print(f"Ошибка вызова: {e}")
