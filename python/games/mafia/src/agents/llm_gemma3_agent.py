import json
import re
import requests
import os
from typing import Dict, Any

from ..models import GameState, AgentAction

class LLMAgent:
    """
    Агент на базе локальной LLM через API сервера Ollama.
    Ожидается, что запущен локальный процесс `ollama serve` с загруженной моделью (например, `gemma3` или `llama3`).
    """
    def __init__(self, name: str, model: str = "gemma3"):
        self.name = name
        self.model = model

    def build_prompt(self, game_state: GameState, my_role: str) -> str:
        # Формируем список живых игроков
        alive_players = [p for p in game_state.players if p.is_alive]
        players_str = ", ".join([f"[ID:{p.id}] {p.name}" for p in alive_players])
        
        # Последние 10 сообщений чата (если есть)
        history = [msg for msg in game_state.chat_history[-10:] if msg.text]
        chat_str = "\n".join([f"{msg.player_name}: {msg.text}" for msg in history])
        if not chat_str:
            chat_str = "No chat history yet."

        prompt = f"""You are playing Mafia game. Your name is '{self.name}'. Your game role is '{my_role}'.
Current phase: {game_state.phase}. Day: {game_state.day}.
Alive players: {players_str}.

Recent chat history (max 10 messages):
{chat_str}

YOUR TASK:
Choose ONE action. 
- If phase is DAY_DISCUSSION, action_type="CHAT_MESSAGE" and provide your message in text_message.
- If phase is DAY_VOTING, action_type="VOTE_KILL" and provide target_id (choose an ID from Alive players).
- If phase is NIGHT_MAFIA and you are MAFIA/DON, action_type="MAFIA_KILL" and provide target_id.
- If phase is NIGHT_SHERIFF and you are SHERIFF, action_type="SHERIFF_CHECK" and provide target_id.
- If phase is NIGHT_DOCTOR and you are DOCTOR, action_type="DOCTOR_HEAL" and provide target_id.
- Otherwise, use action_type="PASS".

OUTPUT FORMAT:
Respond exactly ONLY with valid JSON. No markdown tags, no explanations. Do not include ```json...```:
{{"action_type": "valid_action", "target_id": 1, "text_message": "..."}}
"""
        return prompt

    def parse_action(self, text: str, game_state: GameState) -> AgentAction:
        try:
            # Защита от потенциального мусора в генерации (вырезаем JSON между фигурными скобками)
            match = re.search(r'\{.*?\}', text, re.DOTALL)
            if not match:
                return AgentAction(action_type="PASS")

            json_str = match.group(0)
            data = json.loads(json_str)

            action_type = data.get("action_type", "PASS")
            target_id = data.get("target_id", -1)
            text_message = data.get("text_message", "")
            
            # Базовая валидация экшена
            valid_actions = ["VOTE_KILL", "MAFIA_KILL", "SHERIFF_CHECK", "DOCTOR_HEAL", "CHAT_MESSAGE", "PASS", "DON_CHECK"]
            if action_type not in valid_actions:
                action_type = "PASS"

            # Возвращаем действие через класс AgentAction
            return AgentAction(
                action_type=action_type,
                target_id=int(target_id) if target_id is not None else -1,
                text_message=str(text_message)
            )

        except Exception as e:
            print(f"[LLM JSON Parse Error]: {e}\nRaw output: {text}")
            return AgentAction(action_type="PASS")

    def decide_action(self, game_state: GameState, my_role: str) -> AgentAction:
        prompt = self.build_prompt(game_state, my_role)
        
        api_url = os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")
        
        try:
            response = requests.post(
                api_url,
                json={
                    "model": self.model,
                    "prompt": prompt,
                    "stream": False
                },
                timeout=45 # большой таймаут, на случай долгой работы локальной LLM
            )
            response.raise_for_status()
            
            result = response.json()
            text = result.get("response", "")
            
            print(f"\n[{self.name} - LLM RAW RESPONSE | Mafia]:\n{text}\n{'-'*40}")
            
            return self.parse_action(text, game_state)
            
        except requests.exceptions.ConnectionError:
            print(f"Error: Unable to connect to Ollama at {api_url}.")
            
            # Fallback к localhost на случай запуска без Docker
            if "host.docker.internal" in api_url:
                print("Trying fallback to localhost...")
                try:
                    response = requests.post(
                        "http://localhost:11434/api/generate",
                        json={"model": self.model, "prompt": prompt, "stream": False},
                        timeout=45
                    )
                    response.raise_for_status()
                    
                    result = response.json()
                    text = result.get("response", "")
                    
                    print(f"\n[{self.name} - LLM RAW RESPONSE | Mafia]:\n{text}\n{'-'*40}")
                    
                    return self.parse_action(text, game_state)
                except Exception as fallback_err:
                    print(f"Fallback Error: {fallback_err}")
                    pass

            return AgentAction(action_type="PASS")
        except Exception as e:
            print(f"LLM API Error: {e}")
            return AgentAction(action_type="PASS")
