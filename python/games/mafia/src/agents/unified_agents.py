from typing import Dict, Type
from ..models import GameState, AgentAction
from ..strategies.base_strategy import RoleStrategy
from ..strategies.random_strategies import (
    RandomMafiaStrategy,
    RandomCitizenStrategy,
    RandomSheriffStrategy,
    RandomDoctorStrategy,
)

class BaseAgent:
    def __init__(self, name: str, role_strategies: Dict[str, Type[RoleStrategy]]):
        self.name = name
        self.strategies = {}
        self.strategy_classes = role_strategies

    def _get_strategy(self, role: str) -> RoleStrategy:
        if role not in self.strategies:
            if role in self.strategy_classes:
                self.strategies[role] = self.strategy_classes[role]()
            else:
                self.strategies[role] = RandomCitizenStrategy()
        return self.strategies[role]

    def decide_action(self, game_state: GameState, my_role: str) -> AgentAction:
        strategy = self._get_strategy(my_role.upper())
        return strategy.decide_action(game_state)

class RandomAgent(BaseAgent):
    def __init__(self, name: str):
        super().__init__(
            name,
            {
                "MAFIA": RandomMafiaStrategy,
                "DON": RandomMafiaStrategy,
                "CITIZEN": RandomCitizenStrategy,
                "SHERIFF": RandomSheriffStrategy,
                "DOCTOR": RandomDoctorStrategy,
            },
        )
