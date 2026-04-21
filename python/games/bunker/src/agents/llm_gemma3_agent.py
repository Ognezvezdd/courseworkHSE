import json
import os
import re
import requests
from typing import Dict, Any, List

from ..models import BunkerAction, BunkerState

MODEL_MAP = {
    "gemma3": "gemma3",
    "llama3.2_1b": "llama3.2:1b",
    "llama3.2_3b": "llama3.2:3b",
    "phi3_mini": "phi3:mini",
    "phi4_mini": "phi4-mini",
    "qwen2.5_1.5b": "qwen2.5:1.5b",
}

class BunkerLLMAgent:
    def __init__(self, name: str, model: str = "gemma3", personality: str = "default"):
        self.name = name
        # Мапим короткое имя модели в полное имя для Ollama
        self.model_alias = model
        self.ollama_model = MODEL_MAP.get(model, model)
        self.personality = personality
        self._last_messages = [] # Для анти-спама

    def _build_prompt(self, state: BunkerState) -> str:
        alive = [p for p in state.players if p.is_alive]
        
        # Анонимизация имен в списке живых (в бункере имена обычно передаются как есть, 
        # но для бенчмарка лучше использовать Player_N если мы хотим полной чистоты)
        alive_str = "\n".join(
            [
                f"- [ID:{p.player_id}] Player_{p.player_id}; проф: {p.profession}; "
                f"возраст: {p.age}; здоровье: {p.health}; выж: {p.survival_score}; польз: {p.utility_score}; навыки: {', '.join(p.skills)}"
                for p in alive
            ]
        ) or "нет живых игроков"

        # Анонимизируем историю чата
        recent_chat = state.chat_history[-12:]
        chat_lines = []
        for m in recent_chat:
            name = f"Player_{m.player_id}" if m.player_id != -1 else m.player_name
            chat_lines.append(f"{name}: {m.text}")
        chat_str = "\n".join(chat_lines) or "чат пуст"

        prompt_dir = os.path.join(os.path.dirname(__file__), "../../../../prompts/bunker")
        prompt_file = f"{self.personality}.txt"
        prompt_path = os.path.join(prompt_dir, prompt_file)
        
        if not os.path.exists(prompt_path):
            prompt_path = os.path.join(prompt_dir, "default.txt")

        try:
            with open(prompt_path, "r", encoding="utf-8") as f:
                template = f.read()
            
            my_char = next((p for p in state.players if p.player_id == state.my_player_id), None)
            my_info = "НЕИЗВЕСТНО"
            if my_char:
                my_info = (f"ID: {my_char.player_id}, Ты: Player_{my_char.player_id}, Профессия: {my_char.profession}, "
                          f"Возраст: {my_char.age}, Здоровье: {my_char.health}, "
                          f"Выживаемость: {my_char.survival_score}, Полезность: {my_char.utility_score}, "
                          f"Навыки: {', '.join(my_char.skills)}")

            return template.format(
                name=f"Player_{state.my_player_id}",
                my_player_id=state.my_player_id,
                my_info=my_info,
                phase=state.phase,
                alive_str=alive_str,
                chat_str=chat_str,
                known_info="\n".join(state.known_info) if state.known_info else "Нет дополнительной информации."
            )
        except Exception as e:
            return f"Ты — Player_{state.my_player_id}. Фаза: {state.phase}. Твоя цель — выжить. Выдавай ТОЛЬКО JSON."

    def _parse(self, text: str, state: BunkerState) -> BunkerAction:
        try:
            # Ищем JSON блок
            m = re.search(r"\{.*\}", text, re.DOTALL)
            if not m:
                return self._fallback_action(state)
            
            data = json.loads(m.group(0))
            action_type = str(data.get("action_type", "PASS")).upper()
            target_id = data.get("target_id", -1)
            try:
                target_id = int(target_id)
            except:
                target_id = -1
                
            message = str(data.get("text_message", ""))
            
            # Валидация
            if action_type == "VOTE_EXILE":
                alive_ids = {p.player_id for p in state.players if p.is_alive and p.player_id != state.my_player_id}
                if target_id not in alive_ids:
                    return self._fallback_action(state)
            
            # Анти-спам для сообщений
            if message:
                if message in self._last_messages:
                    message = "" # Не повторяемся
                else:
                    self._last_messages.append(message)
                    if len(self._last_messages) > 5:
                        self._last_messages.pop(0)

            return BunkerAction(action_type=action_type, target_id=target_id, text_message=message)
        except Exception:
            return self._fallback_action(state)

    def _fallback_action(self, state: BunkerState) -> BunkerAction:
        if state.phase == "VOTING":
            alive_others = [p for p in state.players if p.is_alive and p.player_id != state.my_player_id]
            if alive_others:
                import random
                target = random.choice(alive_others).player_id
                return BunkerAction(action_type="VOTE_EXILE", target_id=target)
        return BunkerAction(action_type="PASS")

    def decide(self, state: BunkerState) -> BunkerAction:
        prompt = self._build_prompt(state)
        api_url = os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")
        
        payload = {
            "model": self.ollama_model,
            "prompt": prompt,
            "stream": False,
            "options": {
                "temperature": 0.7,
                "num_predict": 256
            }
        }

        try:
            r = requests.post(api_url, json=payload, timeout=60)
            r.raise_for_status()
            text = r.json().get("response", "")
            return self._parse(text, state)
        except Exception as e:
            print(f"❌ Ошибка вызова Ollama ({self.ollama_model}): {e}")
            return self._fallback_action(state)
