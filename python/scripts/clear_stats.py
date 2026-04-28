
import json
import os

STATS_FILE = os.path.join(os.path.dirname(__file__), "../output/benchmark_stats.json")

def clear_stats():
    empty_stats = {"bunker": {}, "mafia": {}, "tictactoe": {}}
    os.makedirs(os.path.dirname(STATS_FILE), exist_ok=True)
    with open(STATS_FILE, "w", encoding="utf-8") as f:
        json.dump(empty_stats, f, indent=2)
    print(f"✅ Statistics cleared at {STATS_FILE}")

if __name__ == "__main__":
    clear_stats()
