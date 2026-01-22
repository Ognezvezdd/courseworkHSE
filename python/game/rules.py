"""
Модуль правил игры 5×5 крестики-нолики.

Правило победы: 4 символа подряд по горизонтали, вертикали или диагонали.
"""

from typing import Optional

from .board import Board


class Rules:
    """
    Класс правил игры.
    
    Attributes:
        WIN_LENGTH: Количество символов подряд для победы (4).
    """

    WIN_LENGTH = 4

    @classmethod
    def check_winner(cls, board: Board) -> Optional[str]:
        """
        Проверяет, есть ли победитель.
        
        Args:
            board: Объект игрового поля.
            
        Returns:
            "X" или "O" если есть победитель, None если нет.
        """
        grid = board.grid
        size = board.size

        # Проверка горизонталей
        for row in range(size):
            winner = cls._check_line(
                [grid[row][col] for col in range(size)]
            )
            if winner:
                return winner

        # Проверка вертикалей
        for col in range(size):
            winner = cls._check_line(
                [grid[row][col] for row in range(size)]
            )
            if winner:
                return winner

        # Проверка диагоналей (слева-направо сверху-вниз)
        for start_row in range(size - cls.WIN_LENGTH + 1):
            for start_col in range(size - cls.WIN_LENGTH + 1):
                winner = cls._check_line([
                    grid[start_row + i][start_col + i]
                    for i in range(cls.WIN_LENGTH)
                ])
                if winner:
                    return winner

        # Проверка диагоналей (справа-налево сверху-вниз)
        for start_row in range(size - cls.WIN_LENGTH + 1):
            for start_col in range(cls.WIN_LENGTH - 1, size):
                winner = cls._check_line([
                    grid[start_row + i][start_col - i]
                    for i in range(cls.WIN_LENGTH)
                ])
                if winner:
                    return winner

        return None

    @classmethod
    def _check_line(cls, line: list[str]) -> Optional[str]:
        """
        Проверяет линию на наличие WIN_LENGTH одинаковых символов подряд.
        
        Args:
            line: Список символов.
            
        Returns:
            Символ победителя или None.
        """
        count = 1
        for i in range(1, len(line)):
            if line[i] == line[i - 1] and line[i] != Board.EMPTY:
                count += 1
                if count >= cls.WIN_LENGTH:
                    return line[i]
            else:
                count = 1
        return None

    @classmethod
    def is_terminal(cls, board: Board) -> bool:
        """
        Проверяет, завершена ли игра.
        
        Args:
            board: Объект игрового поля.
            
        Returns:
            True если есть победитель или поле заполнено.
        """
        return cls.check_winner(board) is not None or board.is_full()

    @classmethod
    def get_game_result(cls, board: Board) -> Optional[str]:
        """
        Возвращает результат игры.
        
        Args:
            board: Объект игрового поля.
            
        Returns:
            "X", "O", "draw" или None если игра не завершена.
        """
        winner = cls.check_winner(board)
        if winner:
            return winner
        if board.is_full():
            return "draw"
        return None
