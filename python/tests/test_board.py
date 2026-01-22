"""
Тесты игрового поля (Board).
"""

from game.board import Board


class TestBoardInitialization:
    """Тесты инициализации поля."""

    def test_board_default_size(self):
        """Поле создаётся с размером 5x5 по умолчанию."""
        board = Board()
        assert board.size == 5
        assert len(board.grid) == 5
        assert all(len(row) == 5 for row in board.grid)

    def test_board_custom_size(self):
        """Поле создаётся с произвольным размером."""
        board = Board(size=3)
        assert board.size == 3
        assert len(board.grid) == 3

    def test_board_empty_on_init(self):
        """Все клетки пусты после инициализации."""
        board = Board()
        for row in board.grid:
            for cell in row:
                assert cell == Board.EMPTY


class TestBoardMoves:
    """Тесты выполнения ходов."""

    def test_make_valid_move(self):
        """Валидный ход выполняется успешно."""
        board = Board()
        result = board.make_move(0, 0, Board.PLAYER_X)
        assert result is True
        assert board.grid[0][0] == Board.PLAYER_X

    def test_make_move_occupied_cell(self):
        """Ход в занятую клетку отклоняется."""
        board = Board()
        board.make_move(0, 0, Board.PLAYER_X)
        result = board.make_move(0, 0, Board.PLAYER_O)
        assert result is False
        assert board.grid[0][0] == Board.PLAYER_X

    def test_make_move_out_of_bounds(self):
        """Ход за пределы поля отклоняется."""
        board = Board()
        assert board.make_move(-1, 0, Board.PLAYER_X) is False
        assert board.make_move(0, -1, Board.PLAYER_X) is False
        assert board.make_move(5, 0, Board.PLAYER_X) is False
        assert board.make_move(0, 5, Board.PLAYER_X) is False

    def test_make_move_invalid_player(self):
        """Ход с невалидным символом отклоняется."""
        board = Board()
        result = board.make_move(0, 0, "Z")
        assert result is False
        assert board.grid[0][0] == Board.EMPTY


class TestBoardValidMoves:
    """Тесты получения списка допустимых ходов."""

    def test_all_moves_valid_on_empty_board(self):
        """На пустом поле 25 допустимых ходов."""
        board = Board()
        valid_moves = board.get_valid_moves()
        assert len(valid_moves) == 25

    def test_valid_moves_decrease_after_move(self):
        """После хода количество допустимых ходов уменьшается."""
        board = Board()
        board.make_move(2, 2, Board.PLAYER_X)
        valid_moves = board.get_valid_moves()
        assert len(valid_moves) == 24
        assert (2, 2) not in valid_moves

    def test_no_valid_moves_on_full_board(self):
        """На заполненном поле нет допустимых ходов."""
        board = Board(size=2)  # Маленькое поле для теста
        for r in range(2):
            for c in range(2):
                board.make_move(r, c, Board.PLAYER_X)
        assert board.get_valid_moves() == []
        assert board.is_full() is True


class TestBoardCopy:
    """Тесты копирования поля."""

    def test_copy_creates_independent_board(self):
        """Копия поля независима от оригинала."""
        board = Board()
        board.make_move(0, 0, Board.PLAYER_X)

        copy = board.copy()
        copy.make_move(1, 1, Board.PLAYER_O)

        assert board.grid[1][1] == Board.EMPTY
        assert copy.grid[1][1] == Board.PLAYER_O

    def test_to_list_creates_copy(self):
        """to_list() возвращает независимую копию."""
        board = Board()
        board.make_move(0, 0, Board.PLAYER_X)

        state = board.to_list()
        state[0][0] = "Z"

        assert board.grid[0][0] == Board.PLAYER_X


class TestBoardUtilities:
    """Тесты вспомогательных методов."""

    def test_get_cell_valid(self):
        """get_cell возвращает содержимое клетки."""
        board = Board()
        board.make_move(2, 3, Board.PLAYER_O)
        assert board.get_cell(2, 3) == Board.PLAYER_O

    def test_get_cell_invalid(self):
        """get_cell возвращает None для невалидной позиции."""
        board = Board()
        assert board.get_cell(-1, 0) is None
        assert board.get_cell(0, 10) is None

    def test_str_representation(self):
        """Строковое представление содержит поле."""
        board = Board()
        board.make_move(0, 0, Board.PLAYER_X)
        s = str(board)
        assert "X" in s
        assert "." in s
