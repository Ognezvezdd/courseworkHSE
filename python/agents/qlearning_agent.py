"""
Q-learning агент — табличный reinforcement learning (TD(0)).

Использует epsilon-greedy стратегию для баланса exploration/exploitation.
Q-таблица хранится как словарь: state -> {action: q_value}.
"""

import pickle
import random
from pathlib import Path
from typing import Optional

from .base_agent import BaseAgent


class QLearningAgent(BaseAgent):
    """
    Табличный Q-learning агент.

    Параметры обучения:
    - alpha: скорость обучения (learning rate)
    - gamma: коэффициент дисконтирования (discount factor)
    - epsilon: вероятность случайного хода (exploration rate)

    Q-таблица представлена как dict[str, dict[tuple[int, int], float]].
    """

    def __init__(
        self,
        name: str = "QLearningAgent",
        alpha: float = 0.1,
        gamma: float = 0.9,
        epsilon: float = 0.1,
    ):
        super().__init__(name)
        self.alpha = alpha
        self.gamma = gamma
        self.epsilon = epsilon
        self.q_table: dict[str, dict[tuple[int, int], float]] = {}
        self._rng = random.Random()
        self._training = False

    def set_seed(self, seed: int) -> None:
        """Устанавливает seed для генератора случайных чисел."""
        self._rng = random.Random(seed)

    def select_action(self, board: list[list[str]], player_symbol: str) -> tuple[int, int]:
        """
        Выбирает ход на основе Q-таблицы (epsilon-greedy).
        """
        valid_moves = self._get_valid_moves(board)
        if not valid_moves:
            raise ValueError("No valid moves available")

        state = self._board_to_state(board)

        # exploration (Ну то есть расширение чтобы не встать на одном месте)
        if self._training and self._rng.random() < self.epsilon:
            return self._rng.choice(valid_moves)

        # exploration (Ну то есть расширение чтобы не встать на одном месте)
        return self._get_best_action(state, valid_moves)

    def train(
        self,
        opponent: BaseAgent,
        episodes: int = 1000,
        seed: Optional[int] = None,
    ) -> dict[str, int]:
        """
        Обучает агента, играя против противника.
        Возвращает статистику: wins/losses/draws.
        """
        if seed is not None:
            self._rng = random.Random(seed)
            if hasattr(opponent, "set_seed"):
                opponent.set_seed(seed + 1000)

        self._training = True
        stats = {"wins": 0, "losses": 0, "draws": 0}

        for episode in range(episodes):
            play_as_x = episode % 2 == 0
            result = self._play_training_game(opponent, play_as_x)

            if result == "win":
                stats["wins"] += 1
            elif result == "loss":
                stats["losses"] += 1
            else:
                stats["draws"] += 1

        self._training = False
        return stats

    def _play_training_game(self, opponent: BaseAgent, play_as_x: bool) -> str:
        """
        Играет одну обучающую партию, обновляя Q-таблицу по Q-learning (TD(0)).
        """
        board = [["." for _ in range(5)] for _ in range(5)]
        my_symbol = "X" if play_as_x else "O"
        opp_symbol = "O" if play_as_x else "X"
        current = "X"

        # Последний (state, action) нашего агента — чтобы обновить,
        # когда станет известен next_state (после хода соперника или терминала).
        last_state: Optional[str] = None
        last_action: Optional[tuple[int, int]] = None

        while True:
            valid_moves = self._get_valid_moves(board)
            if not valid_moves:
                # Ничья: терминальная награда
                if last_state is not None and last_action is not None:
                    self._q_update(last_state, last_action, reward=0.5, next_state=None, done=True)
                return "draw"

            if current == my_symbol:
                # Перед тем как выбрать новый ход, можно обновить прошлый наш ход,
                # потому что теперь мы находимся в состоянии s' (после ответа соперника).
                if last_state is not None and last_action is not None:
                    next_state = self._board_to_state(board)
                    self._q_update(last_state, last_action, reward=0.0, next_state=next_state, done=False)
                    last_state, last_action = None, None

                state = self._board_to_state(board)
                action = self.select_action(board, my_symbol)

                # применяем наш ход
                board[action[0]][action[1]] = my_symbol

                # проверка терминала после нашего хода
                winner = self._check_winner(board)
                if winner is not None:
                    # мы выиграли
                    self._q_update(state, action, reward=1.0, next_state=None, done=True)
                    return "win"

                # иначе запоминаем переход (обновим после ответа соперника или ничьей)
                last_state, last_action = state, action
                current = opp_symbol
            else:
                # ход соперника
                action = opponent.select_action(board, opp_symbol)
                board[action[0]][action[1]] = opp_symbol

                winner = self._check_winner(board)
                if winner is not None:
                    # соперник выиграл -> для нашего последнего хода терминальная награда 0
                    if last_state is not None and last_action is not None:
                        self._q_update(last_state, last_action, reward=0.0, next_state=None, done=True)
                    return "loss"

                current = my_symbol

    def _q_update(
        self,
        state: str,
        action: tuple[int, int],
        reward: float,
        next_state: Optional[str],
        done: bool,
    ) -> None:
        """
        Стандартное Q-learning обновление:
        Q(s,a) <- Q(s,a) + alpha * (r + gamma*max_a' Q(s',a') - Q(s,a))
        """
        if state not in self.q_table:
            self.q_table[state] = {}

        old_q = self.q_table[state].get(action, 0.0)

        if done or next_state is None:
            target = reward
        else:
            next_qs = self.q_table.get(next_state, {})
            max_next_q = max(next_qs.values(), default=0.0)
            target = reward + self.gamma * max_next_q

        self.q_table[state][action] = old_q + self.alpha * (target - old_q)

    def _get_best_action(self, state: str, valid_moves: list[tuple[int, int]]) -> tuple[int, int]:
        """Возвращает ход с максимальным Q-значением."""
        if state not in self.q_table:
            return self._rng.choice(valid_moves)

        q_values = self.q_table[state]

        best_moves: list[tuple[int, int]] = []
        best_q = float("-inf")

        for move in valid_moves:
            q = q_values.get(move, 0.0)
            if q > best_q:
                best_q = q
                best_moves = [move]
            elif q == best_q:
                best_moves.append(move)

        # при равных Q — детерминировано первый
        return best_moves[0]

    @staticmethod
    def _board_to_state(board: list[list[str]]) -> str:
        """Преобразует поле в строковый ключ."""
        return "".join("".join(row) for row in board)

    @staticmethod
    def _check_winner(board: list[list[str]]) -> Optional[str]:
        """
        Проверяет победителя (4 в ряд на поле 5x5).
        Возвращает "X", "O" или None.
        """
        size = len(board)
        win_len = 4

        for player in ["X", "O"]:
            # Горизонтали и вертикали
            for i in range(size):
                for j in range(size - win_len + 1):
                    if all(board[i][j + k] == player for k in range(win_len)):
                        return player
                    if all(board[j + k][i] == player for k in range(win_len)):
                        return player

            # Диагонали
            for i in range(size - win_len + 1):
                for j in range(size - win_len + 1):
                    if all(board[i + k][j + k] == player for k in range(win_len)):
                        return player
                    if all(board[i + k][j + win_len - 1 - k] == player for k in range(win_len)):
                        return player

        return None

    def save(self, filepath: str) -> None:
        """Сохраняет Q-таблицу в файл."""
        path = Path(filepath)
        path.parent.mkdir(parents=True, exist_ok=True)

        with open(path, "wb") as f:
            pickle.dump(
                {
                    "q_table": self.q_table,
                    "alpha": self.alpha,
                    "gamma": self.gamma,
                    "epsilon": self.epsilon,
                },
                f,
            )

    def load(self, filepath: str) -> None:
        """Загружает Q-таблицу из файла."""
        with open(filepath, "rb") as f:
            data = pickle.load(f)
        self.q_table = data["q_table"]
        self.alpha = data.get("alpha", self.alpha)
        self.gamma = data.get("gamma", self.gamma)
        self.epsilon = data.get("epsilon", self.epsilon)