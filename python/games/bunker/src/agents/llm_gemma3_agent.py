import json
import os
import re
from typing import Dict, Any, List

from ..models import BunkerAction, BunkerState
from games.common.llm_client import LLMClient
from ..logger import bunker_logger

MODEL_MAP = {
    "gemma3": ("ollama", "gemma3"),
    "llama3.2_1b": ("ollama", "llama3.2:1b"),
    "llama3.2_3b": ("ollama", "llama3.2:3b"),
    "phi3_mini": ("ollama", "phi3:mini"),
    "phi4_mini": ("ollama", "phi4-mini"),
    "qwen2.5_1.5b": ("ollama", "qwen2.5:1.5b"),
    "gpt4o_mini": ("openai", "gpt-4o-mini"),
    "gpt4o": ("openai", "gpt-4o"),
}

class BunkerLLMAgent:
    def __init__(self, name: str, model: str = "gemma3", personality: str = "default", openai_api_key: str = None):
        self.name = name
        provider, model_full = MODEL_MAP.get(model, ("ollama", model))
        self.client = LLMClient(provider=provider, model=model_full, api_key=openai_api_key)
        self.personality = personality
        self._last_messages = [] 

    def _build_prompt(self, state: BunkerState) -> str:
        alive = [p for p in state.players if p.is_alive]
        alive_str = "\n".join(
            [
                f"- [ID:{p.player_id}] Player_{p.player_id}; проф: {p.profession}; "
                f"возраст: {p.age}; здоровье: {p.health}; выж: {p.survival_score}; польз: {p.utility_score}; навыки: {', '.join(p.skills)}"
                for p in alive
            ]
        ) or "нет живых игроков"

        recent_chat = state.chat_history[-12:]
        chat_lines = []
        for m in recent_chat:
            p_name = f"Player_{m.player_id}" if m.player_id != -1 else m.player_name
            chat_lines.append(f"{p_name}: {m.text}")
        chat_str = "\n".join(chat_lines) or "чат пуст"

        prompt_dir = os.path.join(os.path.dirname(__file__), "../../../../prompts/bunker")
        prompt_path = os.path.join(prompt_dir, f"{self.personality}.txt")
        
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
        except Exception:
            return f"Ты — Player_{state.my_player_id}. Фаза: {state.phase}. Твоя цель — выжить. Выдавай ТОЛЬКО JSON."

    def decide(self, state: BunkerState) -> BunkerAction:
        prompt = self._build_prompt(state)
        # Добавляем инструкцию про Chain of Thought для логгера
        full_prompt = prompt + "\n\nПеред тем как выдать JSON, напиши кратко свои рассуждения (Thinking Process) в свободном стиле, а затем сам JSON в блоке ```json...```."
        
        response_text = self.client.call(full_prompt)
        
        # Разделяем рассуждение и JSON
        thinking = ""
        json_content = response_text
        
        m = re.search(r"```json(.*?)```", response_text, re.DOTALL)
        if m:
            json_content = m.group(1).strip()
            thinking = response_text[:m.start()].strip()
            # Убираем возможные "рассуждения" за пределами блока
        else:
            # Если блока нет, пробуем найти просто фигурные скобки
            m_braces = re.search(r"\{.*\}", response_text, re.DOTALL)
            if m_braces:
                json_content = m_braces.group(0)
                thinking = response_text[:m_braces.start()].strip()

        # Логируем
        bunker_logger.log_interaction(self.name, prompt, json_content, thinking)
        
        return self._parse(json_content, state)

    def _parse(self, text: str, state: BunkerState) -> BunkerAction:
        try:
            data = json.loads(text)
            action_type = str(data.get("action_type", "PASS")).upper()
            target_id = data.get("target_id", -1)
            try:
                target_id = int(target_id)
            except:
                target_id = -1
                
            message = str(data.get("text_message", ""))
            
            if action_type == "VOTE_EXILE":
                alive_ids = {p.player_id for p in state.players if p.is_alive and p.player_id != state.my_player_id}
                if target_id not in alive_ids:
                    return self._fallback_action(state)
            
            if message:
                if message in self._last_messages:
                    message = "" 
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
