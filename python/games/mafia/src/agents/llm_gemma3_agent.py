import json
import re
import requests
import os
from typing import Dict, Any

from ..models import GameState, AgentAction

ROLE_PROMPT_MAP = {
    "MAFIA": "mafia.txt",
    "DON": "mafia.txt",
    "SHERIFF": "sheriff.txt",
    "DOCTOR": "doctor.txt",
    "CITIZEN": "citizen.txt",
}

MODEL_MAP = {
    "gemma3": "gemma3",
    "llama3.2_1b": "llama3.2:1b",
    "llama3.2_3b": "llama3.2:3b",
    "phi3_mini": "phi3:mini",
    "phi4_mini": "phi4-mini",
    "qwen2.5_1.5b": "qwen2.5:1.5b",
}


class LLMAgent:
    """
    LLM-агент для игры Мафия.
    - Загружает ролевой промпт из python/prompts/mafia/<role>.txt
    - Анонимизирует имена игроков (Player_1, Player_2, ...)
    - Передаёт историю голосований и список выбывших
    - Не раскрывает роли через имена или контекст
    """

    def __init__(self, name: str, model: str = "gemma3"):
        self.name = name

        # Парсим модель из имени агента (llm_gemma3 → gemma3, llm_llama3.2_1b → llama3.2:1b)
        name_lower = name.lower()
        if name_lower.startswith("llm_"):
            model_part = name[4:]
            self.model = MODEL_MAP.get(model_part.lower(), model_part.replace("_", ":"))
        else:
            self.model = model

        # Буфер последних N сообщений для анти-спама
        self._last_messages: list[str] = []

    def _load_prompt_template(self, role: str) -> str:
        filename = ROLE_PROMPT_MAP.get(role.upper(), "default.txt")
        candidates = [
            os.path.join(os.path.dirname(__file__), "../../../../prompts/mafia", filename),
            os.path.join("/app/prompts/mafia", filename),
            os.path.join(os.path.dirname(__file__), "../../../../prompts/mafia/default.txt"),
            "/app/prompts/mafia/default.txt",
        ]
        for path in candidates:
            if os.path.exists(path):
                with open(path, "r", encoding="utf-8") as f:
                    return f.read()
        raise FileNotFoundError(f"Prompt not found for role={role}")

    def build_prompt(self, game_state: GameState, my_role: str) -> str:
        # === Анонимизация ===
        # Строим маппинг id → Player_N (по порядку ID)
        all_players_sorted = sorted(game_state.players, key=lambda p: p.id)
        id_to_alias = {p.id: f"Player_{i+1}" for i, p in enumerate(all_players_sorted)}
        my_alias = id_to_alias.get(game_state.my_id, f"Player_?")

        # Живые игроки (анонимизированные, без ролей)
        alive_players = [p for p in game_state.players if p.is_alive]
        players_str = ", ".join([f"[ID:{p.id}] {id_to_alias[p.id]}" for p in alive_players])

        # === История чата: только ПУБЛИЧНЫЕ сообщения, имена анонимизированы ===
        public_history = [
            msg for msg in game_state.chat_history[-15:]
            if msg.text and msg.is_public and msg.player_id != -1  # Исключаем системные
        ]
        chat_str = "\n".join([
            f"{id_to_alias.get(msg.player_id, msg.player_name)}: {msg.text}"
            for msg in public_history
        ])
        if not chat_str:
            chat_str = "Чат пуст — это первый день."

        # === История голосований ===
        voting_history_str = ""
        if game_state.voting_history:
            lines = []
            for record in game_state.voting_history:
                resolved_votes = []
                for k, v in (record.votes or {}).items():
                    # Пытаемся разрешить ключ как ID, если не выходит — оставляем как есть (уже алиас или имя)
                    try:
                        label = id_to_alias.get(int(k), k)
                    except (ValueError, TypeError):
                        label = k
                    resolved_votes.append(f"{label}: {v} гол.")
                
                votes_str = ", ".join(resolved_votes)
                exiled = record.exiled
                try:
                    # Если exiled это ID в строковом виде
                    exiled = id_to_alias.get(int(exiled), exiled)
                except (ValueError, TypeError):
                    pass
                
                lines.append(f"День {record.day}: {votes_str} → изгнан {exiled if exiled else 'никто'}")
            voting_history_str = "\n".join(lines)
        else:
            voting_history_str = "Голосований ещё не было."

        # === Выбывшие игроки ===
        eliminated_str = (
            ", ".join(game_state.eliminated_players) if game_state.eliminated_players
            else "Никто ещё не выбыл."
        )

        # === Союзники мафии (только для роли MAFIA/DON) ===
        mafia_allies_str = ""
        if my_role.upper() in ("MAFIA", "DON") and game_state.mafia_team_ids:
            allies = [
                id_to_alias[mid] for mid in game_state.mafia_team_ids
                if mid != game_state.my_id and mid in id_to_alias
            ]
            mafia_allies_str = f"Твои союзники в мафии: {', '.join(allies)}" if allies else "Ты единственный член мафии."

        # === Раскрытые роли (через шерифа/дона) ===
        known_roles_str = ""
        if game_state.known_roles:
            lines = [
                f"{id_to_alias.get(int(pid), str(pid))}: {role}"
                for pid, role in game_state.known_roles.items()
            ]
            known_roles_str = "\n".join(lines)
        else:
            known_roles_str = "Нет раскрытых ролей."

        # === Загружаем и форматируем шаблон ===
        try:
            template = self._load_prompt_template(my_role)
            return template.format(
                my_alias=my_alias,
                my_role=my_role,
                game_phase=game_state.phase,
                game_day=game_state.day,
                players_str=players_str,
                chat_str=chat_str,
                voting_history_str=voting_history_str,
                eliminated_str=eliminated_str,
                mafia_allies_str=mafia_allies_str,
                known_roles_str=known_roles_str,
            )
        except Exception as e:
            print(f"[LLMAgent] Error loading prompt: {e}")
            return (
                f"Ты играешь в Мафию. Твоя роль: {my_role}. Твоё имя: {my_alias}.\n"
                f"Фаза: {game_state.phase}, День: {game_state.day}.\n"
                f"Живые игроки: {players_str}.\n"
                f"История чата:\n{chat_str}\n"
                f"Отвечай ТОЛЬКО валидным JSON: "
                '{"action_type": "PASS", "target_id": -1, "text_message": ""}'
            )

    def parse_action(self, text: str, game_state: GameState, my_role: str) -> AgentAction:
        try:
            # Ищем JSON — жадно от первой { до последней }
            match = re.search(r'\{[^{}]*\}', text, re.DOTALL)
            if not match:
                # Пробуем найти JSON с вложенными фигурными скобками
                match = re.search(r'\{.*\}', text, re.DOTALL)
            if not match:
                print(f"[LLMAgent] No JSON found in: {text[:200]}")
                return self._fallback_action(game_state, my_role)

            data = json.loads(match.group(0))
            action_type = str(data.get("action_type", "PASS")).upper()
            target_id = data.get("target_id", -1)
            text_message = str(data.get("text_message", "")).strip()

            valid_actions = {
                "VOTE_KILL", "MAFIA_KILL", "SHERIFF_CHECK",
                "DOCTOR_HEAL", "CHAT_MESSAGE", "PASS", "DON_CHECK"
            }
            if action_type not in valid_actions:
                action_type = "PASS"

            # Анти-спам: если сообщение повторяется — меняем его
            if action_type == "CHAT_MESSAGE":
                text_message = text_message[:300]
                if text_message in self._last_messages[-3:]:
                    text_message = ""
                    action_type = "PASS"
                else:
                    self._last_messages.append(text_message)
                    if len(self._last_messages) > 10:
                        self._last_messages = self._last_messages[-10:]

            return AgentAction(
                action_type=action_type,
                target_id=int(target_id) if target_id is not None else -1,
                text_message=text_message,
            )

        except (json.JSONDecodeError, ValueError, KeyError) as e:
            print(f"[LLMAgent JSON Parse Error]: {e}\nRaw: {text[:300]}")
            return self._fallback_action(game_state, my_role)

    def _fallback_action(self, game_state: GameState, my_role: str) -> AgentAction:
        """Возвращает безопасное действие в зависимости от фазы."""
        import random
        phase = game_state.phase
        alive = [p.id for p in game_state.players if p.is_alive and p.id != game_state.my_id]

        if phase == "DAY_DISCUSSION":
            fallbacks = [
                "Давайте внимательно посмотрим на поведение каждого.",
                "Мне кажется, кто-то из нас нечестен.",
                "Нужно проанализировать вчерашние события.",
                "Кто вёл себя слишком тихо?",
            ]
            return AgentAction(action_type="CHAT_MESSAGE", text_message=random.choice(fallbacks))

        if phase == "DAY_VOTING" and alive:
            return AgentAction(action_type="VOTE_KILL", target_id=random.choice(alive))

        if phase == "NIGHT_MAFIA" and my_role.upper() in ("MAFIA", "DON") and alive:
            return AgentAction(action_type="MAFIA_KILL", target_id=random.choice(alive))

        if phase == "NIGHT_SHERIFF" and my_role.upper() == "SHERIFF" and alive:
            return AgentAction(action_type="SHERIFF_CHECK", target_id=random.choice(alive))

        if phase == "NIGHT_DOCTOR" and my_role.upper() == "DOCTOR" and alive:
            target = game_state.my_id if game_state.my_id in [p.id for p in game_state.players] else (alive[0] if alive else -1)
            return AgentAction(action_type="DOCTOR_HEAL", target_id=target)

        return AgentAction(action_type="PASS")

    def decide_action(self, game_state: GameState, my_role: str) -> AgentAction:
        prompt = self.build_prompt(game_state, my_role)

        api_url = os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")

        payload = {
            "model": self.model,
            "prompt": prompt,
            "stream": False,
            "options": {
                "temperature": 0.7,
                "top_p": 0.9,
                "num_predict": 256,
            }
        }

        def try_request(url: str) -> str | None:
            try:
                r = requests.post(url, json=payload, timeout=60)
                r.raise_for_status()
                return r.json().get("response", "")
            except Exception as err:
                print(f"[LLMAgent] Request to {url} failed: {err}")
                return None

        text = try_request(api_url)
        if text is None and "host.docker.internal" in api_url:
            print("[LLMAgent] Trying localhost fallback...")
            text = try_request("http://localhost:11434/api/generate")

        if text is None:
            return self._fallback_action(game_state, my_role)

        print(f"\n[{self.name} ({self.model}) | {my_role} | {game_state.phase}]:\n{text[:500]}\n{'─'*50}")
        return self.parse_action(text, game_state, my_role)
