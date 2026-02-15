"""
Модуль игрового поля 5×5 для крестиков-ноликов.

Поле представлено как 2D-список строк:
- "." — пустая клетка
- "X" — крестик
- "O" — нолик
"""

import copy
from typing import Optional


class Board:
    """
    Класс игрового поля 5×5.
    
    Attributes:
        size: Размер поля (5).
        grid: 2D-список состояния клеток.
    """

    EMPTY = "."
    PLAYER_X = "X"
    PLAYER_O = "O"

    def __init__(self, size: int = 5):
        """
        Инициализирует пустое игровое поле.
        
        Args:
            size: Размер поля (по умолчанию 5).
        """
        self.size = size
        self.grid: list[list[str]] = [
            [self.EMPTY for _ in range(size)]
            for _ in range(size)
        ]

    def make_move(self, row: int, col: int, player: str) -> bool:
        """
        Выполняет ход игрока.
        
        Args:
            row: Номер строки (0-indexed).
            col: Номер столбца (0-indexed).
            player: Символ игрока ("X" или "O").
            
        Returns:
            True если ход выполнен, False если клетка занята или вне поля.
        """
        if not self._is_valid_position(row, col):
            return False
        if self.grid[row][col] != self.EMPTY:
            return False
        if player not in (self.PLAYER_X, self.PLAYER_O):
            return False

        self.grid[row][col] = player
        return True

    def get_valid_moves(self) -> list[tuple[int, int]]:
        """
        Возвращает список допустимых ходов (пустых клеток).
        
        Returns:
            Список кортежей (row, col) для каждой пустой клетки.
        """
        moves = []
        for row in range(self.size):
            for col in range(self.size):
                if self.grid[row][col] == self.EMPTY:
                    moves.append((row, col))
        return moves

    def is_full(self) -> bool:
        """
        Проверяет, заполнено ли поле полностью.
        
        Returns:
            True если нет пустых клеток.
        """
        return len(self.get_valid_moves()) == 0

    def get_cell(self, row: int, col: int) -> Optional[str]:
        """
        Возвращает содержимое клетки.
        
        Args:
            row: Номер строки.
            col: Номер столбца.
            
        Returns:
            Символ в клетке или None если позиция невалидна.
        """
        if not self._is_valid_position(row, col):
            return None
        return self.grid[row][col]

    def copy(self) -> "Board":
        """
        Создаёт глубокую копию поля.
        
        Returns:
            Новый объект Board с идентичным состоянием.
        """
        new_board = Board(self.size)
        new_board.grid = copy.deepcopy(self.grid)
        return new_board

    def to_list(self) -> list[list[str]]:
        """
        Возвращает копию состояния поля как 2D-список.
        
        Returns:
            Копия grid для использования в slides.
        """
        return copy.deepcopy(self.grid)

    def _is_valid_position(self, row: int, col: int) -> bool:
        """Проверяет, находится ли позиция в пределах поля."""
        return 0 <= row < self.size and 0 <= col < self.size

    def __str__(self) -> str:
        """Строковое представление поля для отладки."""
        lines = []
        header = "  " + " ".join(str(i) for i in range(self.size))
        lines.append(header)
        for i, row in enumerate(self.grid):
            lines.append(f"{i} " + " ".join(row))
        return "\n".join(lines)

    def __repr__(self) -> str:
        return f"Board(size={self.size})"
