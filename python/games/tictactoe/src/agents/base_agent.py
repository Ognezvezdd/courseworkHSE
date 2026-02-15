"""
Базовый абстрактный класс агента.

Все агенты должны наследовать этот класс и реализовывать метод select_action().
"""

from abc import ABC, abstractmethod


class BaseAgent(ABC):
    """
    Абстрактный базовый класс для AI-агентов.
    
    Каждый агент должен реализовать метод select_action(),
    который принимает состояние игры и возвращает ход.
    """

    def __init__(self, name: str = "BaseAgent"):
        """
        Инициализация агента.
        
        Args:
            name: Имя агента для идентификации.
        """
        self.name = name

    @abstractmethod
    def select_action(
            self,
            board: list[list[str]],
            player_symbol: str
    ) -> tuple[int, int]:
        """
        Выбирает ход на основе текущего состояния поля.
        
        Args:
            board: 2D-список состояния поля.
                   "." — пустая клетка, "X" — крестик, "O" — нолик.
            player_symbol: Символ текущего игрока ("X" или "O").
            
        Returns:
            Кортеж (row, col) — координаты выбранного хода.
            Ход должен быть валидным (клетка должна быть пустой).
        """
        pass

    def set_seed(self, seed: int) -> None:
        """
        Устанавливает seed для генератора случайных чисел.
        
        Этот метод вызывается перед началом игры для обеспечения
        воспроизводимости. Агенты с детерминированной логикой
        могут не реализовывать этот метод.
        
        Args:
            seed: Значение seed.
        """
        pass

    @staticmethod
    def _get_valid_moves(board: list[list[str]]) -> list[tuple[int, int]]:
        """
        Вспомогательный метод: возвращает список допустимых ходов.

        Args:
            board: 2D-список состояния поля.

        Returns:
            Список кортежей (row, col) для пустых клеток.
        """
        moves = []
        for row in range(len(board)):
            for col in range(len(board[0])):
                if board[row][col] == ".":
                    moves.append((row, col))
        return moves

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}(name='{self.name}')"
