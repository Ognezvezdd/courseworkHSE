"""
Тесты AI-агентов.
"""

import pytest

from agents.base_agent import BaseAgent
from agents.heuristic_agent import HeuristicAgent
from agents.qlearning_agent import QLearningAgent
from agents.random_agent import RandomAgent


class TestAgentInterface:
    """Тесты единого интерфейса агентов."""

    def test_all_agents_inherit_base(self):
        """Все агенты наследуют BaseAgent."""
        assert issubclass(RandomAgent, BaseAgent)
        assert issubclass(HeuristicAgent, BaseAgent)
        assert issubclass(QLearningAgent, BaseAgent)

    def test_all_agents_have_select_action(self):
        """Все агенты реализуют select_action."""
        agents = [RandomAgent(), HeuristicAgent(), QLearningAgent()]

        for agent in agents:
            assert hasattr(agent, 'select_action')
            assert callable(agent.select_action)

    def test_all_agents_have_set_seed(self):
        """Все агенты имеют метод set_seed."""
        agents = [RandomAgent(), HeuristicAgent(), QLearningAgent()]

        for agent in agents:
            assert hasattr(agent, 'set_seed')
            # set_seed не должен вызывать ошибку
            agent.set_seed(42)


class TestRandomAgent:
    """Тесты случайного агента."""

    @pytest.fixture
    def empty_board(self):
        return [["." for _ in range(5)] for _ in range(5)]

    @pytest.fixture
    def almost_full_board(self):
        board = [["X" if (r + c) % 2 == 0 else "O" for c in range(5)] for r in range(5)]
        board[0][0] = "."  # Оставляем одну пустую клетку
        return board

    def test_returns_valid_move(self, empty_board):
        """Агент возвращает валидный ход."""
        agent = RandomAgent()
        action = agent.select_action(empty_board, "X")

        assert isinstance(action, tuple)
        assert len(action) == 2
        row, col = action
        assert 0 <= row < 5
        assert 0 <= col < 5
        assert empty_board[row][col] == "."

    def test_returns_only_valid_move(self, almost_full_board):
        """При одном допустимом ходе возвращает его."""
        agent = RandomAgent()
        action = agent.select_action(almost_full_board, "X")

        assert action == (0, 0)

    def test_raises_on_full_board(self):
        """Выбрасывает исключение на полном поле."""
        agent = RandomAgent()
        full_board = [["X" for _ in range(5)] for _ in range(5)]

        with pytest.raises(ValueError, match="No valid moves"):
            agent.select_action(full_board, "X")


class TestHeuristicAgent:
    """Тесты эвристического агента."""

    def test_wins_if_possible(self):
        """Агент выигрывает, если может."""
        agent = HeuristicAgent()
        board = [["." for _ in range(5)] for _ in range(5)]
        # 3 X подряд в первой строке
        board[0][0] = "X"
        board[0][1] = "X"
        board[0][2] = "X"

        action = agent.select_action(board, "X")

        # Должен поставить 4-й X
        assert action in [(0, 3), (0, 4)] or action == (0, 3)  # (0, 3) выиграет

    def test_blocks_opponent_win(self):
        """Агент блокирует победу противника."""
        agent = HeuristicAgent()
        board = [["." for _ in range(5)] for _ in range(5)]
        # 3 O подряд в первой строке
        board[0][0] = "O"
        board[0][1] = "O"
        board[0][2] = "O"

        action = agent.select_action(board, "X")

        # Должен заблокировать
        assert action == (0, 3)

    def test_prefers_center(self):
        """Агент предпочитает центр на пустом поле."""
        agent = HeuristicAgent()
        board = [["." for _ in range(5)] for _ in range(5)]

        action = agent.select_action(board, "X")

        assert action == (2, 2)

    def test_deterministic(self):
        """Агент детерминирован."""
        agent = HeuristicAgent()
        board = [["." for _ in range(5)] for _ in range(5)]
        board[0][0] = "X"

        action1 = agent.select_action(board, "O")
        action2 = agent.select_action(board, "O")

        assert action1 == action2


class TestQLearningAgent:
    """Тесты Q-learning агента."""

    @pytest.fixture
    def agent(self):
        return QLearningAgent(epsilon=0.0)  # Без exploration для тестов

    @pytest.fixture
    def empty_board(self):
        return [["." for _ in range(5)] for _ in range(5)]

    def test_returns_valid_move(self, agent, empty_board):
        """Агент возвращает валидный ход."""
        action = agent.select_action(empty_board, "X")

        assert isinstance(action, tuple)
        row, col = action
        assert empty_board[row][col] == "."

    def test_training_updates_q_table(self):
        """Обучение обновляет Q-таблицу."""
        agent = QLearningAgent()
        opponent = RandomAgent()

        agent.train(opponent, episodes=10, seed=42)

        assert len(agent.q_table) > 0

    def test_training_returns_stats(self):
        """Обучение возвращает статистику."""
        agent = QLearningAgent()
        opponent = RandomAgent()

        stats = agent.train(opponent, episodes=10, seed=42)

        assert "wins" in stats
        assert "losses" in stats
        assert "draws" in stats
        assert stats["wins"] + stats["losses"] + stats["draws"] == 10

    def test_seed_reproducibility(self, empty_board):
        """Агент воспроизводим с seed."""
        agent1 = QLearningAgent()
        agent1.set_seed(42)

        agent2 = QLearningAgent()
        agent2.set_seed(42)

        action1 = agent1.select_action(empty_board, "X")
        action2 = agent2.select_action(empty_board, "X")

        assert action1 == action2
