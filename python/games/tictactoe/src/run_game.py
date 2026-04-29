"""Legacy CLI wrapper for Tic-Tac-Toe game execution."""

import json
import argparse

from games.tictactoe.src.agents.heuristic_agent import HeuristicAgent
from games.tictactoe.src.agents.qlearning_agent import QLearningAgent
from games.tictactoe.src.agents.random_agent import RandomAgent
from games.tictactoe.src.game.engine import run_game

AGENT_CLASSES = {
    "random": RandomAgent,
    "heuristic": HeuristicAgent,
    "qlearning": QLearningAgent,
}


def create_agent(agent_type: str) -> object:
    """Создает агента по типу."""
    if agent_type not in AGENT_CLASSES:
        raise ValueError(f"Unknown agent type: {agent_type}")
    
    return AGENT_CLASSES[agent_type]()


def main():
    parser = argparse.ArgumentParser(description="Запуск игры AI-агентов")
    parser.add_argument("--agent-x", required=True, help="Агент X")
    parser.add_argument("--agent-o", required=True, help="Агент O")
    parser.add_argument("--seed", type=int, default=0, help="Seed для воспроизводимости")
    parser.add_argument("--json", action="store_true", help="Вывод в JSON формате")
    
    args = parser.parse_args()
    
    try:
        agent_x = create_agent(args.agent_x)
        agent_o = create_agent(args.agent_o)

        seed = args.seed if args.seed != 0 else None

        slides = run_game(agent_x, agent_o, seed=seed)

        result = {
            "winner": slides[-1]["winner"] if slides[-1]["winner"] else "draw",
            "steps": len(slides),
            "slides": slides
        }

        print(json.dumps(result, indent=2))
        
    except Exception as e:
        error_result = {
            "error": str(e),
            "winner": "error",
            "steps": 0
        }
        print(json.dumps(error_result, indent=2))


if __name__ == "__main__":
    main()
