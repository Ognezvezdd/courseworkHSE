"""
Визуализация игрового поля с помощью matplotlib.
"""

from pathlib import Path
from typing import Optional

import matplotlib.patches as patches
import matplotlib.pyplot as plt
from matplotlib.colors import to_rgba


class BoardVisualizer:
    """
    Класс для визуализации состояния игрового поля.
    
    Рисует поле с крестиками и ноликами в стиле настольной игры.
    """

    # Цветовая схема
    COLORS = {
        'background': '#2C3E50',  # Тёмно-синий фон
        'grid': '#ECF0F1',  # Светлые линии сетки
        'cell': '#34495E',  # Цвет клеток
        'x': '#E74C3C',  # Красный для X
        'o': '#3498DB',  # Синий для O
        'highlight': '#F39C12',  # Жёлтый для подсветки последнего хода
        'text': '#FFFFFF',  # Белый текст
    }

    def __init__(self, cell_size: float = 1.0, line_width: float = 3.0):
        """
        Инициализация визуализатора.
        
        Args:
            cell_size: Размер клетки в дюймах.
            line_width: Толщина линий.
        """
        self.cell_size = cell_size
        self.line_width = line_width

    def draw_board(
            self,
            board: list[list[str]],
            last_action: Optional[tuple[int, int]] = None,
            step: Optional[int] = None,
            winner: Optional[str] = None,
            ax: Optional[plt.Axes] = None
    ) -> plt.Figure:
        """
        Рисует игровое поле.
        
        Args:
            board: 2D-список состояния поля.
            last_action: Координаты последнего хода (row, col).
            step: Номер хода.
            winner: Победитель ("X", "O", "draw" или None).
            ax: Существующие оси matplotlib (опционально).
            
        Returns:
            Figure объект matplotlib.
        """
        size = len(board)
        fig_size = size * self.cell_size + 1

        if ax is None:
            fig, ax = plt.subplots(figsize=(fig_size, fig_size + 0.5))
        else:
            fig = ax.get_figure()

        ax.set_facecolor(self.COLORS['background'])
        fig.patch.set_facecolor(self.COLORS['background'])

        # Рисуем клетки
        for row in range(size):
            for col in range(size):
                # Подсветка последнего хода
                if last_action and (row, col) == last_action:
                    cell_color = self.COLORS['highlight']
                    alpha = 0.3
                else:
                    cell_color = self.COLORS['cell']
                    alpha = 1.0

                rect = patches.Rectangle(
                    (col, size - 1 - row),
                    1, 1,
                    linewidth=2,
                    edgecolor=self.COLORS['grid'],
                    facecolor=to_rgba(cell_color, alpha)
                )
                ax.add_patch(rect)

                # Рисуем X или O
                cell = board[row][col]
                center_x = col + 0.5
                center_y = size - 1 - row + 0.5

                if cell == 'X':
                    self._draw_x(ax, center_x, center_y)
                elif cell == 'O':
                    self._draw_o(ax, center_x, center_y)

        # Настройка осей
        ax.set_xlim(0, size)
        ax.set_ylim(0, size)
        ax.set_aspect('equal')
        ax.axis('off')

        # Заголовок
        title_parts = []
        if step is not None:
            title_parts.append(f"Ход {step}")
        if winner:
            if winner == 'draw':
                title_parts.append("Ничья!")
            else:
                title_parts.append(f"Победа {winner}!")

        if title_parts:
            ax.set_title(
                " — ".join(title_parts),
                color=self.COLORS['text'],
                fontsize=14,
                fontweight='bold',
                pad=10
            )

        plt.tight_layout()
        return fig

    def _draw_x(self, ax: plt.Axes, cx: float, cy: float, size: float = 0.35):
        """Рисует крестик."""
        ax.plot(
            [cx - size, cx + size],
            [cy - size, cy + size],
            color=self.COLORS['x'],
            linewidth=self.line_width * 2,
            solid_capstyle='round'
        )
        ax.plot(
            [cx - size, cx + size],
            [cy + size, cy - size],
            color=self.COLORS['x'],
            linewidth=self.line_width * 2,
            solid_capstyle='round'
        )

    def _draw_o(self, ax: plt.Axes, cx: float, cy: float, radius: float = 0.32):
        """Рисует нолик."""
        circle = patches.Circle(
            (cx, cy),
            radius,
            linewidth=self.line_width * 2,
            edgecolor=self.COLORS['o'],
            facecolor='none'
        )
        ax.add_patch(circle)

    def save(
            self,
            board: list[list[str]],
            filepath: str,
            last_action: Optional[tuple[int, int]] = None,
            step: Optional[int] = None,
            winner: Optional[str] = None,
            dpi: int = 150
    ) -> str:
        """
        Сохраняет изображение поля в файл.
        
        Args:
            board: Состояние поля.
            filepath: Путь для сохранения.
            last_action: Координаты последнего хода.
            step: Номер хода.
            winner: Победитель.
            dpi: Разрешение изображения.
            
        Returns:
            Путь к сохранённому файлу.
        """
        fig = self.draw_board(board, last_action, step, winner)

        path = Path(filepath)
        path.parent.mkdir(parents=True, exist_ok=True)

        fig.savefig(
            filepath,
            dpi=dpi,
            bbox_inches='tight',
            facecolor=fig.get_facecolor(),
            edgecolor='none'
        )
        plt.close(fig)

        return str(path.absolute())
