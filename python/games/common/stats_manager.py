"""
StatsManager — хранит статистику бенчмарка в JSON-файле (без БД).

Структура файла:
{
  "bunker": {
    "bunker_llm_rational": {
      "games": 10, "survived": 6, "exiled": 4
    },
    ...
  },
  "mafia": {
    "llm_gemma3": {
      "games": 5, "wins_as_citizen": 3, "wins_as_mafia": 1,
      "losses_as_citizen": 1, "losses_as_mafia": 0,
      "total_wins": 4, "total_losses": 1
    },
    ...
  },
  "tictactoe": {
    "random": {"games": 20, "wins": 8, "draws": 4, "losses": 8},
    ...
  }
}
"""

import json
import os
import threading
from typing import Dict, List

STATS_FILE = os.path.join(os.path.dirname(__file__), "../../output/benchmark_stats.json")
_lock = threading.Lock()


def _load() -> dict:
    if not os.path.exists(STATS_FILE):
        return {"bunker": {}, "mafia": {}, "tictactoe": {}}
    try:
        with open(STATS_FILE, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {"bunker": {}, "mafia": {}, "tictactoe": {}}


def _save(data: dict):
    os.makedirs(os.path.dirname(STATS_FILE), exist_ok=True)
    with open(STATS_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


# ─────────────────────────────────────────────────────────────────
# Bunker
# ─────────────────────────────────────────────────────────────────

def record_bunker(survivors: List[str], exiled: List[str]):
    """
    survivors: список agent_type-имён агентов, выживших.
    exiled:    список agent_type-имён агентов, изгнанных.
    """
    with _lock:
        data = _load()
        b = data.setdefault("bunker", {})

        for name in survivors:
            entry = b.setdefault(name, {"games": 0, "survived": 0, "exiled": 0})
            entry["games"] += 1
            entry["survived"] += 1

        for name in exiled:
            entry = b.setdefault(name, {"games": 0, "survived": 0, "exiled": 0})
            entry["games"] += 1
            entry["exiled"] += 1

        _save(data)


# ─────────────────────────────────────────────────────────────────
# Mafia
# ─────────────────────────────────────────────────────────────────

def record_mafia(winner: str, mafia_agents: List[str], citizen_agents: List[str]):
    """
    winner:          "mafia" | "citizens"
    mafia_agents:    agent_type списки агентов мафии
    citizen_agents:  agent_type списки мирных агентов
    """
    with _lock:
        data = _load()
        m = data.setdefault("mafia", {})

        mafia_won = (winner == "mafia")

        for name in mafia_agents:
            entry = m.setdefault(name, {
                "games": 0,
                "wins_as_mafia": 0, "losses_as_mafia": 0,
                "wins_as_citizen": 0, "losses_as_citizen": 0,
            })
            entry["games"] += 1
            if mafia_won:
                entry["wins_as_mafia"] += 1
            else:
                entry["losses_as_mafia"] += 1

        for name in citizen_agents:
            entry = m.setdefault(name, {
                "games": 0,
                "wins_as_mafia": 0, "losses_as_mafia": 0,
                "wins_as_citizen": 0, "losses_as_citizen": 0,
            })
            entry["games"] += 1
            if not mafia_won:
                entry["wins_as_citizen"] += 1
            else:
                entry["losses_as_citizen"] += 1

        _save(data)


# ─────────────────────────────────────────────────────────────────
# TicTacToe
# ─────────────────────────────────────────────────────────────────

def record_tictactoe(agent_x: str, agent_o: str, winner: str):
    """winner: "X" | "O" | "draw" """
    with _lock:
        data = _load()
        t = data.setdefault("tictactoe", {})

        def _update(name, result):
            entry = t.setdefault(name, {"games": 0, "wins": 0, "draws": 0, "losses": 0})
            entry["games"] += 1
            if result == "win":
                entry["wins"] += 1
            elif result == "draw":
                entry["draws"] += 1
            else:
                entry["losses"] += 1

        if winner == "X":
            _update(agent_x, "win")
            _update(agent_o, "loss")
        elif winner == "O":
            _update(agent_x, "loss")
            _update(agent_o, "win")
        else:
            _update(agent_x, "draw")
            _update(agent_o, "draw")

        _save(data)


# ─────────────────────────────────────────────────────────────────
# Report
# ─────────────────────────────────────────────────────────────────

def get_report() -> dict:
    with _lock:
        data = _load()

    report = {"bunker": [], "mafia": [], "tictactoe": []}

    # Bunker — сортируем по survival_rate
    for name, s in data.get("bunker", {}).items():
        games = s.get("games", 0) or 1
        report["bunker"].append({
            "agent": name,
            "games": s.get("games", 0),
            "survived": s.get("survived", 0),
            "exiled": s.get("exiled", 0),
            "survival_rate": round(s.get("survived", 0) / games * 100, 1),
        })
    report["bunker"].sort(key=lambda x: x["survival_rate"], reverse=True)

    # Mafia — считаем общий win rate
    for name, s in data.get("mafia", {}).items():
        games = s.get("games", 0) or 1
        wins = s.get("wins_as_mafia", 0) + s.get("wins_as_citizen", 0)
        report["mafia"].append({
            "agent": name,
            "games": s.get("games", 0),
            "wins": wins,
            "win_rate": round(wins / games * 100, 1),
            "wins_as_mafia": s.get("wins_as_mafia", 0),
            "wins_as_citizen": s.get("wins_as_citizen", 0),
        })
    report["mafia"].sort(key=lambda x: x["win_rate"], reverse=True)

    # TicTacToe
    for name, s in data.get("tictactoe", {}).items():
        games = s.get("games", 0) or 1
        report["tictactoe"].append({
            "agent": name,
            "games": s.get("games", 0),
            "wins": s.get("wins", 0),
            "draws": s.get("draws", 0),
            "losses": s.get("losses", 0),
            "win_rate": round(s.get("wins", 0) / games * 100, 1),
        })
    report["tictactoe"].sort(key=lambda x: x["win_rate"], reverse=True)

    return report
