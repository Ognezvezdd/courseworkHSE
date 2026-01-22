"""
Игровой движок — управление ходом игры.

Основная функция run_game() запускает партию между двумя агентами
и возвращает пошаговую историю (slides).
"""

import random
from typing import Optional, Protocol

from .board import Board
from .rules import Rules


class Agent(Protocol):
    """Протокол для типизации агентов."""

    def select_action(
            self,
            board: list[list[str]],
            player_symbol: str
    ) -> tuple[int, int]:
        """Выбирает ход (row, col)."""
        ...

    def set_seed(self, seed: int) -> None:
        """Устанавливает seed для воспроизводимости."""
        ...


def run_game(
        agent_x: Agent,
        agent_o: Agent,
        seed: Optional[int] = None
) -> list[dict]:
    """
    Запускает игру между двумя агентами.
    
    Args:
        agent_x: Агент, играющий за X (ходит первым).
        agent_o: Агент, играющий за O.
        seed: Seed для воспроизводимости (передаётся агентам).
        
    Returns:
        Список slides — история игры по шагам.
        Каждый slide — словарь с информацией о ходе.
    """
    # Тут ставим seed для агентов
    if seed is not None:
        random.seed(seed)
        if hasattr(agent_x, 'set_seed'):
            agent_x.set_seed(seed)
        if hasattr(agent_o, 'set_seed'):
            # Используем другой seed для O, чтобы агенты были независимы, но поведение все еще детерминированное
            agent_o.set_seed(seed + 1)

    board = Board()
    slides: list[dict] = []
    step = 0

    agents = {
        Board.PLAYER_X: agent_x,
        Board.PLAYER_O: agent_o
    }
    current_player = Board.PLAYER_X

    while not Rules.is_terminal(board):
        agent = agents[current_player]

        # Агент получает копию состояния поля
        board_state = board.to_list()
        action = agent.select_action(board_state, current_player)

        # Валидация хода
        row, col = action
        valid_moves = board.get_valid_moves()

        if action not in valid_moves:
            # Если агент вернул невалидный ход — ошибка
            raise ValueError(
                f"Agent {current_player} returned invalid move {action}. "
                f"Valid moves: {valid_moves}"
            )

        # Выполнение хода
        board.make_move(row, col, current_player)
        step += 1

        # Проверка состояния после хода
        is_terminal = Rules.is_terminal(board)
        winner = Rules.get_game_result(board) if is_terminal else None

        # Формирование slide
        slide = {
            "step": step,
            "current_player": current_player,
            "action": action,
            "board": board.to_list(),
            "is_terminal": is_terminal,
            "winner": winner
        }
        slides.append(slide)

        # Смена игрока
        current_player = (
            Board.PLAYER_O if current_player == Board.PLAYER_X
            else Board.PLAYER_X
        )

    return slides


def format_slides(slides: list[dict]) -> str:
    """
    Форматирует slides для вывода в консоль.
    
    Args:
        slides: Список slides из run_game().
        
    Returns:
        Отформатированная строка для печати.
    """
    output = []

    for slide in slides:
        output.append(f"\n=== Step {slide['step']} ===")
        output.append(f"Player: {slide['current_player']}")
        output.append(f"Action: {slide['action']}")
        output.append("")

        # Отрисовка поля
        board = slide['board']
        header = "  " + " ".join(str(i) for i in range(len(board)))
        output.append(header)
        for i, row in enumerate(board):
            output.append(f"{i} " + " ".join(row))

        if slide['is_terminal']:
            output.append("")
            if slide['winner'] == "draw":
                output.append("🤝 Result: Draw!")
            else:
                output.append(f"🏆 Winner: {slide['winner']}!")

    return "\n".join(output)
