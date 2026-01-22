"""
Рендеринг полной игры из JSON slides.
"""

import json
from pathlib import Path
from typing import Union

import matplotlib.pyplot as plt

from .board_visualizer import BoardVisualizer


class GameRenderer:
    """
    Рендерер игровых партий.
    
    Генерирует изображения для каждого шага игры или создаёт
    сводное изображение всей партии.
    """

    def __init__(self, visualizer: BoardVisualizer = None):
        """
        Инициализация рендерера.
        
        Args:
            visualizer: Экземпляр BoardVisualizer (создаётся автоматически).
        """
        self.visualizer = visualizer or BoardVisualizer()

    def render_slides(
            self,
            slides: Union[list[dict], str],
            output_dir: str = "output",
            prefix: str = "step",
            dpi: int = 150
    ) -> list[str]:
        """
        Рендерит все шаги игры в отдельные изображения.
        
        Args:
            slides: Список slides или путь к JSON-файлу.
            output_dir: Директория для сохранения изображений.
            prefix: Префикс имени файлов.
            dpi: Разрешение изображений.
            
        Returns:
            Список путей к созданным файлам.
        """
        # Загрузка из JSON если передан путь
        if isinstance(slides, str):
            slides = self._load_json(slides)

        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)

        created_files = []

        for slide in slides:
            step = slide["step"]
            board = slide["board"]
            action = tuple(slide["action"]) if slide["action"] else None
            winner = slide.get("winner")

            filename = f"{prefix}_{step:03d}.png"
            filepath = output_path / filename

            self.visualizer.save(
                board=board,
                filepath=str(filepath),
                last_action=action,
                step=step,
                winner=winner,
                dpi=dpi
            )

            created_files.append(str(filepath))

        return created_files

    def render_summary(
            self,
            slides: Union[list[dict], str],
            filepath: str = "game_summary.png",
            max_steps: int = 9,
            dpi: int = 150
    ) -> str:
        """
        Создаёт сводное изображение с ключевыми моментами игры.
        
        Args:
            slides: Список slides или путь к JSON-файлу.
            filepath: Путь для сохранения.
            max_steps: Максимальное количество шагов на изображении.
            dpi: Разрешение.
            
        Returns:
            Путь к созданному файлу.
        """
        if isinstance(slides, str):
            slides = self._load_json(slides)

        total_steps = len(slides)

        # Выбираем шаги для отображения
        if total_steps <= max_steps:
            selected_slides = slides
        else:
            # Равномерно распределяем + обязательно последний
            indices = list(range(0, total_steps - 1, total_steps // (max_steps - 1)))
            indices = indices[:max_steps - 1] + [total_steps - 1]
            selected_slides = [slides[i] for i in indices]

        n = len(selected_slides)
        cols = min(n, 3)
        rows = (n + cols - 1) // cols

        fig, axes = plt.subplots(
            rows, cols,
            figsize=(cols * 3, rows * 3.5),
            facecolor=BoardVisualizer.COLORS['background']
        )

        # Обработка случая одной строки/столбца
        if rows == 1 and cols == 1:
            axes = [[axes]]
        elif rows == 1:
            axes = [axes]
        elif cols == 1:
            axes = [[ax] for ax in axes]

        for idx, slide in enumerate(selected_slides):
            row_idx = idx // cols
            col_idx = idx % cols
            ax = axes[row_idx][col_idx]

            board = slide["board"]
            action = tuple(slide["action"]) if slide["action"] else None
            step = slide["step"]
            winner = slide.get("winner")

            self.visualizer.draw_board(
                board=board,
                last_action=action,
                step=step,
                winner=winner,
                ax=ax
            )

        # Скрываем пустые ячейки
        for idx in range(n, rows * cols):
            row_idx = idx // cols
            col_idx = idx % cols
            axes[row_idx][col_idx].axis('off')

        plt.tight_layout()

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

    def render_final(
            self,
            slides: Union[list[dict], str],
            filepath: str = "final_state.png",
            dpi: int = 150
    ) -> str:
        """
        Рендерит только финальное состояние игры.
        
        Args:
            slides: Список slides или путь к JSON-файлу.
            filepath: Путь для сохранения.
            dpi: Разрешение.
            
        Returns:
            Путь к созданному файлу.
        """
        if isinstance(slides, str):
            slides = self._load_json(slides)

        final_slide = slides[-1]

        return self.visualizer.save(
            board=final_slide["board"],
            filepath=filepath,
            last_action=tuple(final_slide["action"]) if final_slide["action"] else None,
            step=final_slide["step"],
            winner=final_slide.get("winner"),
            dpi=dpi
        )

    def _load_json(self, filepath: str) -> list[dict]:
        """Загружает slides из JSON-файла."""
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)

        # Конвертируем action из list в tuple
        for slide in data:
            if isinstance(slide.get("action"), list):
                slide["action"] = tuple(slide["action"])

        return data
