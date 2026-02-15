#!/usr/bin/env python3
"""
CLI-скрипт для визуализации игровых партий.

Использование:
    python3 visualize.py game.json --output output/
    python3 visualize.py game.json --summary
    python3 visualize.py game.json --final
"""

import argparse
import json
import sys
from pathlib import Path

from visualization import GameRenderer, BoardVisualizer


def main():
    """Основная функция CLI."""
    parser = argparse.ArgumentParser(
        description="Визуализация игровых партий AI-агентов"
    )
    parser.add_argument(
        "input",
        type=str,
        help="Путь к JSON-файлу с slides или '-' для stdin"
    )
    parser.add_argument(
        "--output", "-o",
        type=str,
        default="output",
        help="Директория для сохранения (по умолчанию: output)"
    )
    parser.add_argument(
        "--summary", "-s",
        action="store_true",
        help="Создать сводное изображение игры"
    )
    parser.add_argument(
        "--final", "-f",
        action="store_true",
        help="Сохранить только финальное состояние"
    )
    parser.add_argument(
        "--all", "-a",
        action="store_true",
        help="Сохранить все шаги игры"
    )
    parser.add_argument(
        "--dpi",
        type=int,
        default=300,
        help="Разрешение изображений (по умолчанию: 300)"
    )
    
    args = parser.parse_args()
    
    # Загрузка данных
    if args.input == '-':
        slides = json.load(sys.stdin)
    else:
        with open(args.input, 'r', encoding='utf-8') as f:
            slides = json.load(f)
    
    # Конвертация actions
    for slide in slides:
        if isinstance(slide.get("action"), list):
            slide["action"] = tuple(slide["action"])
    
    renderer = GameRenderer()
    output_dir = Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    created_files = []
    
    # По умолчанию создаём summary
    if not args.summary and not args.final and not args.all:
        args.summary = True
    
    if args.all:
        print(f"📸 Рендеринг всех {len(slides)} шагов...")
        files = renderer.render_slides(slides, str(output_dir), dpi=args.dpi)
        created_files.extend(files)
        print(f"   Создано файлов: {len(files)}")
    
    if args.summary:
        filepath = str(output_dir / "game_summary.png")
        print(f"📊 Создание сводного изображения...")
        renderer.render_summary(slides, filepath, dpi=args.dpi)
        created_files.append(filepath)
        print(f"   Сохранено: {filepath}")
    
    if args.final:
        filepath = str(output_dir / "final_state.png")
        print(f"🏁 Рендеринг финального состояния...")
        renderer.render_final(slides, filepath, dpi=args.dpi)
        created_files.append(filepath)
        print(f"   Сохранено: {filepath}")
    
    print(f"\n✅ Готово! Создано файлов: {len(created_files)}")
    
    # Информация о результате игры
    final = slides[-1]
    if final.get("winner"):
        if final["winner"] == "draw":
            print("🤝 Результат: Ничья")
        else:
            print(f"🏆 Победитель: {final['winner']}")


if __name__ == "__main__":
    main()
