"""
Случайный агент — выбирает случайный допустимый ход.

Используется как baseline и для тестирования воспроизводимости.
"""

import random

from .base_agent import BaseAgent


class RandomAgent(BaseAgent):
    """
    Агент, выбирающий случайный допустимый ход.
    
    Поддерживает seed для воспроизводимости результатов.
    """

    def __init__(self, name: str = "RandomAgent"):
        """
        Инициализация случайного агента.
        
        Args:
            name: Имя агента.
        """
        super().__init__(name)
        self._rng = random.Random()

    def select_action(
            self,
            board: list[list[str]],
            player_symbol: str
    ) -> tuple[int, int]:
        """
        Выбирает случайный допустимый ход.
        
        Args:
            board: 2D-список состояния поля.
            player_symbol: Символ игрока (не используется).
            
        Returns:
            Случайные координаты (row, col) пустой клетки.
            
        Raises:
            ValueError: Если нет допустимых ходов.
        """
        valid_moves = self._get_valid_moves(board)

        if not valid_moves:
            raise ValueError("No valid moves available")

        return self._rng.choice(valid_moves)

    def set_seed(self, seed: int) -> None:
        """
        Устанавливает seed для генератора случайных чисел.
        
        Args:
            seed: Значение seed.
        """
        self._rng = random.Random(seed)
