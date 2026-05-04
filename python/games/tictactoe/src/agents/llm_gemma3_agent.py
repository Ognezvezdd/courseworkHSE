import json
import re
import os

from .base_agent import BaseAgent
from games.common.llm_client import LLMClient

class LLMAgent(BaseAgent):
    """
    Агент на базе локальной LLM через API Ollama.
    """
    def __init__(self, name: str = "LLMAgent", model: str = "gemma3"):
        super().__init__(name)
        self.model = model
        self.client = LLMClient(provider="ollama", model=model)
        self.last_used_fallback = False
        self.last_fallback_reason = ""

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

        result = self.client.call_result(prompt)
        self.last_used_fallback = result.used_fallback
        self.last_fallback_reason = result.fallback_reason

        try:
            match = re.search(r'\{.*?\}', result.text, re.DOTALL)
            if match:
                data = json.loads(match.group(0))
                row = data.get("row")
                col = data.get("col")
                if row is not None and col is not None:
                    move = (int(row), int(col))
                    if move in valid_moves:
                        return move
        except (json.JSONDecodeError, TypeError, ValueError):
            pass

        self.last_used_fallback = True
        if not self.last_fallback_reason:
            self.last_fallback_reason = "invalid_llm_move"
        return valid_moves[0]
