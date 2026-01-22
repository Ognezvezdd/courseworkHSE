"""
Игровая среда: 5×5 крестики-нолики.
"""

from .board import Board
from .engine import run_game
from .rules import Rules

__all__ = ["Board", "Rules", "run_game"]
