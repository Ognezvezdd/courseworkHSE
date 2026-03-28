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
        
        prompt = f"""You are playing Tic-Tac-Toe on a 5x5 board. Your symbol is '{player_symbol}'.
To win, you need 4 symbols in a row (horizontally, vertically, or diagonally).

Current board state:
{board_str}
Valid moves available (row, column):
{moves_str}

YOUR TASK:
Choose ONE valid move from the list of valid moves to maximize your chance of winning or blocking the opponent.
You must respond with ONLY a valid JSON object. No explanation, no markdown tags. Do not write ```json .
Format:
{{"row": <int>, "col": <int>}}
"""
        return prompt

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
