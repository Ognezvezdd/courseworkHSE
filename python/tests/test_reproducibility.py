"""
Тесты воспроизводимости результатов.
"""

from agents.heuristic_agent import HeuristicAgent
from agents.qlearning_agent import QLearningAgent
from agents.random_agent import RandomAgent
from game.engine import run_game


class TestReproducibility:
    """Тесты воспроизводимости игр при фиксированном seed."""

    def test_same_seed_same_result_random_vs_random(self):
        """Одинаковый seed даёт идентичный результат (Random vs Random)."""
        agent_x1 = RandomAgent()
        agent_o1 = RandomAgent()
        slides1 = run_game(agent_x1, agent_o1, seed=42)

        agent_x2 = RandomAgent()
        agent_o2 = RandomAgent()
        slides2 = run_game(agent_x2, agent_o2, seed=42)

        assert len(slides1) == len(slides2)
        for s1, s2 in zip(slides1, slides2):
            assert s1["step"] == s2["step"]
            assert s1["current_player"] == s2["current_player"]
            assert s1["action"] == s2["action"]
            assert s1["board"] == s2["board"]
            assert s1["winner"] == s2["winner"]

    def test_same_seed_same_result_heuristic_vs_random(self):
        """Одинаковый seed даёт идентичный результат (Heuristic vs Random)."""
        slides1 = run_game(HeuristicAgent(), RandomAgent(), seed=123)
        slides2 = run_game(HeuristicAgent(), RandomAgent(), seed=123)

        assert len(slides1) == len(slides2)
        for s1, s2 in zip(slides1, slides2):
            assert s1["action"] == s2["action"]
            assert s1["winner"] == s2["winner"]

    def test_different_seed_different_result(self):
        """Разные seed дают разные результаты."""
        agent_x1 = RandomAgent()
        agent_o1 = RandomAgent()
        slides1 = run_game(agent_x1, agent_o1, seed=42)

        agent_x2 = RandomAgent()
        agent_o2 = RandomAgent()
        slides2 = run_game(agent_x2, agent_o2, seed=999)

        # Хотя бы один ход должен отличаться (с высокой вероятностью)
        actions1 = [s["action"] for s in slides1]
        actions2 = [s["action"] for s in slides2]

        # Допускаем, что игры могут отличаться длиной
        assert actions1 != actions2 or len(slides1) != len(slides2)

    def test_reproducibility_multiple_runs(self):
        """Множественные запуски с одним seed идентичны."""
        seed = 777
        results = []

        for _ in range(5):
            slides = run_game(RandomAgent(), RandomAgent(), seed=seed)
            winner = slides[-1]["winner"]
            actions = tuple(s["action"] for s in slides)
            results.append((winner, actions))

        # Все результаты должны быть одинаковыми
        assert all(r == results[0] for r in results)

    def test_heuristic_vs_heuristic_deterministic(self):
        """Heuristic vs Heuristic всегда одинаков (детерминированные агенты)."""
        slides1 = run_game(HeuristicAgent(), HeuristicAgent())
        slides2 = run_game(HeuristicAgent(), HeuristicAgent())

        assert len(slides1) == len(slides2)
        for s1, s2 in zip(slides1, slides2):
            assert s1["action"] == s2["action"]


class TestQlearningReproducibility:
    """Тесты воспроизводимости Q-learning агента."""

    def test_training_reproducible(self):
        """Обучение воспроизводимо с seed."""
        agent1 = QLearningAgent()
        opponent1 = RandomAgent()
        stats1 = agent1.train(opponent1, episodes=50, seed=42)

        agent2 = QLearningAgent()
        opponent2 = RandomAgent()
        stats2 = agent2.train(opponent2, episodes=50, seed=42)

        assert stats1 == stats2

    def test_trained_agent_reproducible(self):
        """Обученный агент даёт воспроизводимые результаты."""
        # Обучаем агента
        agent = QLearningAgent(epsilon=0.0)
        opponent = RandomAgent()
        agent.train(opponent, episodes=100, seed=42)

        # Играем несколько игр с одним seed
        slides1 = run_game(agent, RandomAgent(), seed=100)

        # Создаём нового агента с теми же параметрами
        agent2 = QLearningAgent(epsilon=0.0)
        agent2.q_table = agent.q_table.copy()

        slides2 = run_game(agent2, RandomAgent(), seed=100)

        assert len(slides1) == len(slides2)
        for s1, s2 in zip(slides1, slides2):
            assert s1["action"] == s2["action"]
