#!/usr/bin/env python3
"""
Демонстрационный скрипт запуска игры между AI-агентами.

Использование:
    python3 run.py --agent-x random --agent-o heuristic --seed 42
    python3 run.py --agent-x qlearning --agent-o random --seed 42 --train 1000
"""

import argparse
import json

from agents.heuristic_agent import HeuristicAgent
from agents.qlearning_agent import QLearningAgent
from agents.random_agent import RandomAgent
from agents.llm_gemma3_agent import LLMAgent
from game.engine import run_game, format_slides

AGENT_CLASSES = {
    "random": RandomAgent,
    "heuristic": HeuristicAgent,
    "qlearning": QLearningAgent,
    "llm": LLMAgent,
}


def create_agent(agent_type: str, name: str) -> object:
    """
    Создаёт агента по типу.
    
    Args:
        agent_type: Тип агента (random, heuristic, qlearning).
        name: Имя агента.
        
    Returns:
        Экземпляр агента.
    """
    if agent_type not in AGENT_CLASSES:
        raise ValueError(f"Unknown agent type: {agent_type}. "
                         f"Available: {list(AGENT_CLASSES.keys())}")

    return AGENT_CLASSES[agent_type](name=name)


def main():
    """Основная функция демонстрации."""
    parser = argparse.ArgumentParser(
        description="Демонстрация игры AI-агентов в крестики-нолики 5×5"
    )
    parser.add_argument(
        "--agent-x",
        type=str,
        default="random",
        choices=list(AGENT_CLASSES.keys()),
        help="Тип агента X (по умолчанию: random)"
    )
    parser.add_argument(
        "--agent-o",
        type=str,
        default="heuristic",
        choices=list(AGENT_CLASSES.keys()),
        help="Тип агента O (по умолчанию: heuristic)"
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=None,
        help="Seed для воспроизводимости"
    )
    parser.add_argument(
        "--train",
        type=int,
        default=0,
        help="Количество эпизодов обучения для Q-learning агента"
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Вывести slides в формате JSON"
    )
    parser.add_argument(
        "--games",
        type=int,
        default=1,
        help="Количество игр для запуска"
    )

    args = parser.parse_args()

    # Создание агентов
    agent_x = create_agent(args.agent_x, f"Agent_X ({args.agent_x})")
    agent_o = create_agent(args.agent_o, f"Agent_O ({args.agent_o})")

    # Вывод заголовка только если не JSON режим
    if not args.json:
        print(f"🎮 AI Agents Platform — Крестики-нолики 5×5")
        print(f"=" * 50)
        print(f"Агент X: {agent_x}")
        print(f"Агент O: {agent_o}")
        print(f"Seed: {args.seed}")
        print()

    # Обучение Q-learning агента, если требуется
    if args.train > 0:
        if isinstance(agent_x, QLearningAgent):
            print(f"🧠 Обучение агента X ({args.train} эпизодов)...")
            stats = agent_x.train(RandomAgent(), episodes=args.train, seed=args.seed)
            print(f"   Результаты: {stats}")
            print()

        if isinstance(agent_o, QLearningAgent):
            print(f"🧠 Обучение агента O ({args.train} эпизодов)...")
            stats = agent_o.train(RandomAgent(), episodes=args.train, seed=args.seed)
            print(f"   Результаты: {stats}")
            print()

    # Статистика для множественных игр
    if args.games > 1:
        stats = {"X": 0, "O": 0, "draw": 0}

        for i in range(args.games):
            seed = args.seed + i if args.seed is not None else None
            slides = run_game(agent_x, agent_o, seed=seed)
            winner = slides[-1]["winner"]
            stats[winner] += 1

        print(f"📊 Статистика за {args.games} игр:")
        print(f"   X побед: {stats['X']} ({100 * stats['X'] / args.games:.1f}%)")
        print(f"   O побед: {stats['O']} ({100 * stats['O'] / args.games:.1f}%)")
        print(f"   Ничьих: {stats['draw']} ({100 * stats['draw'] / args.games:.1f}%)")
        return

    # Запуск одной игры
    slides = run_game(agent_x, agent_o, seed=args.seed)

    if args.json:
        # Конвертируем tuples в lists для JSON
        json_slides = []
        for slide in slides:
            json_slide = slide.copy()
            json_slide["action"] = list(slide["action"])
            json_slides.append(json_slide)

        print(json.dumps(json_slides, indent=2, ensure_ascii=False))
    else:
        print(format_slides(slides))
        print()
        print(f"📊 Всего ходов: {len(slides)}")


if __name__ == "__main__":
    main()
