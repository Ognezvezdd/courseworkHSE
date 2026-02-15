"""
Эвристический агент — использует правила для выбора хода.

Стратегия:
1. Если можно выиграть за 1 ход — выигрывает.
2. Если противник может выиграть за 1 ход — блокирует.
3. Предпочитает центр поля.
4. Предпочитает ходы рядом с существующими фигурами.
5. Если ничего из вышеперечисленного — выбирает первый допустимый ход.
"""

from .base_agent import BaseAgent


class HeuristicAgent(BaseAgent):
    """
    Эвристический агент с детерминированной стратегией.
    
    Не требует seed, так как логика полностью детерминирована.
    """

    WIN_LENGTH = 4  # Должно совпадать с Rules.WIN_LENGTH

    def __init__(self, name: str = "HeuristicAgent"):
        """
        Инициализация эвристического агента.
        
        Args:
            name: Имя агента.
        """
        super().__init__(name)

    def select_action(
            self,
            board: list[list[str]],
            player_symbol: str
    ) -> tuple[int, int]:
        """
        Выбирает ход на основе эвристик.
        
        Args:
            board: 2D-список состояния поля.
            player_symbol: Символ текущего игрока ("X" или "O").
            
        Returns:
            Координаты (row, col) выбранного хода.
        """
        valid_moves = self._get_valid_moves(board)

        if not valid_moves:
            raise ValueError("No valid moves available")

        opponent = "O" if player_symbol == "X" else "X"

        # 1. Проверяем возможность выигрыша
        winning_move = self._find_winning_move(board, player_symbol)
        if winning_move:
            return winning_move

        # 2. Блокируем выигрыш противника
        blocking_move = self._find_winning_move(board, opponent)
        if blocking_move:
            return blocking_move

        # 3. Предпочитаем центр
        size = len(board)
        center = size // 2
        if (center, center) in valid_moves:
            return center, center

        # 4. Предпочитаем ходы рядом с существующими фигурами
        adjacent_moves = self._get_adjacent_moves(board, player_symbol, valid_moves)
        if adjacent_moves:
            return adjacent_moves[0]

        # 5. Fallback — первый допустимый ход
        return valid_moves[0]

    def _find_winning_move(
            self,
            board: list[list[str]],
            player: str
    ) -> tuple[int, int] | None:
        """
        Ищет ход, приводящий к победе.

        Args:
            board: Состояние поля.
            player: Символ игрока.

        Returns:
            Координаты выигрышного хода или None.
        """
        valid_moves = self._get_valid_moves(board)

        for move in valid_moves:
            # Симулируем ход
            test_board = [row[:] for row in board]
            test_board[move[0]][move[1]] = player

            if self._check_win(test_board, player):
                return move

        return None

    def _check_win(self, board: list[list[str]], player: str) -> bool:
        """
        Проверяет, выиграл ли игрок.
        
        Args:
            board: Состояние поля.
            player: Символ игрока.
            
        Returns:
            True если игрок выиграл.
        """
        size = len(board)
        win_len = self.WIN_LENGTH

        # Проверка горизонталей и вертикалей
        for i in range(size):
            for j in range(size - win_len + 1):
                # Горизонталь
                if all(board[i][j + k] == player for k in range(win_len)):
                    return True
                # Вертикаль
                if all(board[j + k][i] == player for k in range(win_len)):
                    return True

        # Проверка диагоналей
        for i in range(size - win_len + 1):
            for j in range(size - win_len + 1):
                # Главная диагональ
                if all(board[i + k][j + k] == player for k in range(win_len)):
                    return True
                # Побочная диагональ
                if all(board[i + k][j + win_len - 1 - k] == player for k in range(win_len)):
                    return True

        return False

    @staticmethod
    def _get_adjacent_moves(
            board: list[list[str]],
            player: str,
            valid_moves: list[tuple[int, int]]
    ) -> list[tuple[int, int]]:
        """
        Возвращает ходы, соседствующие с фигурами игрока.
        
        Args:
            board: Состояние поля.
            player: Символ игрока.
            valid_moves: Список допустимых ходов.
            
        Returns:
            Отсортированный список соседних ходов.
        """
        size = len(board)
        adjacent = []

        for move in valid_moves:
            row, col = move
            # Проверяем 8 соседних клеток
            for dr in [-1, 0, 1]:
                for dc in [-1, 0, 1]:
                    if dr == 0 and dc == 0:
                        continue
                    nr, nc = row + dr, col + dc
                    if 0 <= nr < size and 0 <= nc < size:
                        if board[nr][nc] == player:
                            adjacent.append(move)
                            break
                else:
                    continue
                break

        return adjacent
