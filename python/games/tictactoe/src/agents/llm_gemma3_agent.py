import json
import re
import requests
import os
from typing import Optional

from .base_agent import BaseAgent

class LLMAgent(BaseAgent):
    """
    Агент на базе локальной LLM через API Ollama.
    """
    def __init__(self, name: str = "LLMAgent", model: str = "gemma3"):
        super().__init__(name)
        self.model = model

    def build_prompt(self, board: list[list[str]], player_symbol: str) -> str:
        valid_moves = self._get_valid_moves(board)
        
        # Форматируем доску для более понятного вида для LLM
        board_str = ""
        for r_idx, row in enumerate(board):
            board_str += f"Row {r_idx}: " + " ".join(row) + "\n"
        
        moves_str = ", ".join([f"[{r}, {c}]" for r, c in valid_moves])
        
        prompt_path = os.path.join(os.path.dirname(__file__), "../../../../prompts/tictactoe/default.txt")
        if not os.path.exists(prompt_path):
            # Fallback if path logic differs in docker
            prompt_path = "/app/prompts/tictactoe/default.txt"
            
        try:
            with open(prompt_path, "r", encoding="utf-8") as f:
                template = f.read()
            return template.format(player_symbol=player_symbol, board_str=board_str, moves_str=moves_str)
        except Exception as e:
            print(f"Error loading prompt from {prompt_path}: {e}")
            # Minimal hardcoded fallback
            return f"Play Tic-Tac-Toe. Symbol: {player_symbol}. Board:\n{board_str}\nMoves: {moves_str}"

    def select_action(self, board: list[list[str]], player_symbol: str) -> tuple[int, int]:
        valid_moves = self._get_valid_moves(board)
        if not valid_moves:
            return (-1, -1)
            
        prompt = self.build_prompt(board, player_symbol)
        
        api_url = os.getenv("OLLAMA_API_URL", "http://host.docker.internal:11434/api/generate")
        
        try:
            response = requests.post(
                api_url,
                json={
                    "model": self.model,
                    "prompt": prompt,
                    "stream": False
                },
                timeout=45
            )
            response.raise_for_status()
            text = response.json().get("response", "")
            
            print(f"\n[{self.name} - LLM RAW RESPONSE | TicTacToe]:\n{text}\n{'-'*40}")
            
            # Поиск JSON внутри ответа
            match = re.search(r'\{.*?\}', text, re.DOTALL)
            if match:
                data = json.loads(match.group(0))
                row = data.get("row")
                col = data.get("col")
                if row is not None and col is not None:
                    if (int(row), int(col)) in valid_moves:
                        return (int(row), int(col))
                
            # Если LLM сгенерировала некорректный ход или JSON, делаем случайный-валидный "fallback"
            print(f"LLM made bad move: {text}")
            return valid_moves[0]
            
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
                    text = response.json().get("response", "")
                    match = re.search(r'\{.*?\}', text, re.DOTALL)
                    if match:
                        data = json.loads(match.group(0))
                        row = data.get("row")
                        col = data.get("col")
                        if row is not None and col is not None:
                            if (int(row), int(col)) in valid_moves:
                                return (int(row), int(col))
                except Exception as fallback_err:
                    print(f"Fallback Error: {fallback_err}")
                    pass

            return valid_moves[0]
        except Exception as e:
            print(f"[LLM Agent Error] {e}")
            return valid_moves[0]
