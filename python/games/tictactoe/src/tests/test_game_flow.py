"""
Тесты хода игры и правил.
"""

from agents.heuristic_agent import HeuristicAgent
from agents.random_agent import RandomAgent
from game.board import Board
from game.engine import run_game
from game.rules import Rules


class TestRulesWinDetection:
    """Тесты определения победителя."""

    def test_horizontal_win(self):
        """Победа по горизонтали (4 подряд)."""
        board = Board()
        for col in range(4):
            board.make_move(0, col, Board.PLAYER_X)

        assert Rules.check_winner(board) == Board.PLAYER_X

    def test_vertical_win(self):
        """Победа по вертикали (4 подряд)."""
        board = Board()
        for row in range(4):
            board.make_move(row, 0, Board.PLAYER_O)

        assert Rules.check_winner(board) == Board.PLAYER_O

    def test_diagonal_win_main(self):
        """Победа по главной диагонали."""
        board = Board()
        for i in range(4):
            board.make_move(i, i, Board.PLAYER_X)

        assert Rules.check_winner(board) == Board.PLAYER_X

    def test_diagonal_win_anti(self):
        """Победа по побочной диагонали."""
        board = Board()
        for i in range(4):
            board.make_move(i, 3 - i, Board.PLAYER_O)

        assert Rules.check_winner(board) == Board.PLAYER_O

    def test_no_winner_three_in_row(self):
        """3 подряд — не победа."""
        board = Board()
        for col in range(3):
            board.make_move(0, col, Board.PLAYER_X)

        assert Rules.check_winner(board) is None

    def test_no_winner_empty_board(self):
        """На пустом поле нет победителя."""
        board = Board()
        assert Rules.check_winner(board) is None


class TestRulesTerminal:
    """Тесты завершения игры."""

    def test_game_terminal_on_win(self):
        """Игра завершена при победе."""
        board = Board()
        for col in range(4):
            board.make_move(0, col, Board.PLAYER_X)

        assert Rules.is_terminal(board) is True

    def test_game_terminal_on_full_board(self):
        """Игра завершена при заполненном поле."""
        board = Board(size=2)
        board.make_move(0, 0, Board.PLAYER_X)
        board.make_move(0, 1, Board.PLAYER_O)
        board.make_move(1, 0, Board.PLAYER_O)
        board.make_move(1, 1, Board.PLAYER_X)

        assert Rules.is_terminal(board) is True

    def test_game_not_terminal_in_progress(self):
        """Игра не завершена в процессе."""
        board = Board()
        board.make_move(0, 0, Board.PLAYER_X)

        assert Rules.is_terminal(board) is False


class TestRulesGameResult:
    """Тесты результата игры."""

    def test_result_winner_x(self):
        """Результат — победа X."""
        board = Board()
        for col in range(4):
            board.make_move(0, col, Board.PLAYER_X)

        assert Rules.get_game_result(board) == Board.PLAYER_X

    def test_result_draw(self):
        """Результат — ничья (заполненное поле без победителя)."""
        board = Board(size=2)
        # Заполняем так, чтобы не было победителя
        board.make_move(0, 0, Board.PLAYER_X)
        board.make_move(0, 1, Board.PLAYER_O)
        board.make_move(1, 0, Board.PLAYER_O)
        board.make_move(1, 1, Board.PLAYER_X)

        assert Rules.get_game_result(board) == "draw"

    def test_result_none_in_progress(self):
        """Результат None, пока игра идёт."""
        board = Board()
        assert Rules.get_game_result(board) is None


class TestGameEngine:
    """Тесты игрового движка run_game()."""

    def test_run_game_returns_slides(self):
        """run_game возвращает непустой список slides."""
        agent_x = RandomAgent()
        agent_o = RandomAgent()

        slides = run_game(agent_x, agent_o, seed=42)

        assert isinstance(slides, list)
        assert len(slides) > 0

    def test_slides_structure(self):
        """Каждый slide имеет правильную структуру."""
        agent_x = RandomAgent()
        agent_o = RandomAgent()

        slides = run_game(agent_x, agent_o, seed=42)

        required_keys = {"step", "current_player", "action", "board", "is_terminal", "winner"}

        for slide in slides:
            assert required_keys.issubset(slide.keys())
            assert isinstance(slide["step"], int)
            assert slide["current_player"] in ("X", "O")
            assert isinstance(slide["action"], tuple)
            assert len(slide["action"]) == 2
            assert isinstance(slide["board"], list)
            assert isinstance(slide["is_terminal"], bool)

    def test_game_ends_properly(self):
        """Последний slide отмечен как терминальный."""
        agent_x = RandomAgent()
        agent_o = RandomAgent()

        slides = run_game(agent_x, agent_o, seed=42)

        assert slides[-1]["is_terminal"] is True
        assert slides[-1]["winner"] in ("X", "O", "draw")

    def test_game_with_heuristic_agent(self):
        """Игра с эвристическим агентом завершается корректно."""
        agent_x = HeuristicAgent()
        agent_o = RandomAgent()

        slides = run_game(agent_x, agent_o, seed=42)

        assert slides[-1]["is_terminal"] is True

    def test_step_numbers_sequential(self):
        """Номера шагов последовательны."""
        agent_x = RandomAgent()
        agent_o = RandomAgent()

        slides = run_game(agent_x, agent_o, seed=42)

        for i, slide in enumerate(slides):
            assert slide["step"] == i + 1

    def test_players_alternate(self):
        """Игроки чередуются (X начинает)."""
        agent_x = RandomAgent()
        agent_o = RandomAgent()

        slides = run_game(agent_x, agent_o, seed=42)

        expected_player = "X"
        for slide in slides:
            assert slide["current_player"] == expected_player
            expected_player = "O" if expected_player == "X" else "X"
