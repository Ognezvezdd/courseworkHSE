import json
import os
import re
import requests

from ..models import BunkerAction, BunkerState


class BunkerLLMAgent:
    def __init__(self, name: str, model: str = "gemma3"):
        self.name = name
        self.model = model

    def _build_prompt(self, state: BunkerState) -> str:
        alive = [p for p in state.players if p.is_alive]
        alive_str = "\n".join(
            [
                f"- id={p.player_id}; name={p.player_name}; profession={p.profession}; "
                f"age={p.age}; health={p.health}; skills={', '.join(p.skills)}"
                for p in alive
            ]
        ) or "none"

        recent_chat = state.chat_history[-8:]
        chat_str = "\n".join([f"{m.player_name}: {m.text}" for m in recent_chat]) or "none"

        return f"""You are a player in the Bunker survival social game.
Your name is "{self.name}". Your player_id is {state.my_player_id}.
Current phase: {state.phase}.

Alive players:
{alive_str}

Recent chat:
{chat_str}

Known info:
{state.known_info}

Task:
- If phase is VOTING, choose exactly one alive target_id (not yourself) and return action_type "VOTE_EXILE".
- If phase is DISCUSSION, return action_type "PASS" and provide a short persuasive text_message.
- Otherwise return action_type "PASS".

Output strict JSON only:
{{"action_type":"VOTE_EXILE|PASS","target_id":-1,"text_message":"..."}}
"""

    def _parse(self, text: str) -> BunkerAction:
        try:
            m = re.search(r"\{.*\}", text, re.DOTALL)
            if not m:
                return BunkerAction(action_type="PASS")
            data = json.loads(m.group(0))
            action_type = str(data.get("action_type", "PASS")).upper()
            if action_type not in {"VOTE_EXILE", "PASS"}:
                action_type = "PASS"
            target_id = int(data.get("target_id", -1))
            message = str(data.get("text_message", ""))
            return BunkerAction(action_type=action_type, target_id=target_id, text_message=message)
        except Exception:
            return BunkerAction(action_type="PASS")

    def decide(self, state: BunkerState) -> BunkerAction:
        prompt = self._build_prompt(state)
        api_url = os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")
        payload = {"model": self.model, "prompt": prompt, "stream": False}

        try:
            r = requests.post(api_url, json=payload, timeout=45)
            r.raise_for_status()
            text = r.json().get("response", "")
            return self._parse(text)
        except Exception:
            # fallback для локального запуска без Docker
            try:
                r = requests.post("http://localhost:11434/api/generate", json=payload, timeout=45)
                r.raise_for_status()
                text = r.json().get("response", "")
                return self._parse(text)
            except Exception:
                return BunkerAction(action_type="PASS")

