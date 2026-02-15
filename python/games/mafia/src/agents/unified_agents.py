
from typing import Dict, Any, Type
from mafia.models import GameState, AgentAction, PlayerState
from mafia.strategies.base_strategy import RoleStrategy
from mafia.strategies.random_strategies import RandomMafiaStrategy, RandomCitizenStrategy, RandomSheriffStrategy, RandomDoctorStrategy
from mafia.strategies.rl_strategy import LearningStrategy

class BaseAgent:
    """
    Базовый унифицированный агент.
    Имеет набор стратегий (`RoleStrategy`) для каждой роли.
    Выбирает нужную стратегию в рантайме.
    """
    def __init__(self, name: str, role_strategies: Dict[str, Type[RoleStrategy]]):
        self.name = name
        self.strategies = {} # Cache created strategies
        self.strategy_classes = role_strategies

    def _get_strategy(self, role: str) -> RoleStrategy:
        if role not in self.strategies:
            if role in self.strategy_classes:
                self.strategies[role] = self.strategy_classes[role]()
            else:
                # Fallback для неизвестных ролей (например, мирный)
                self.strategies[role] = RandomCitizenStrategy() 
        return self.strategies[role]

    def decide_action(self, game_state: GameState, my_role: str) -> AgentAction:
        strategy = self._get_strategy(my_role.upper())
        # Можно добавить логику "общей памяти" (если агент stateful)
        # self.memory.update(game_state) 
        
        return strategy.decide_action(game_state)

class RandomAgent(BaseAgent):
    """Агент, который играет случайно за любую роль."""
    def __init__(self, name: str):
        super().__init__(name, {
            "MAFIA": RandomMafiaStrategy,
            "DON": RandomMafiaStrategy,
            "CITIZEN": RandomCitizenStrategy,
            "SHERIFF": RandomSheriffStrategy,
            "DOCTOR": RandomDoctorStrategy
        })

class RLAgent(BaseAgent):
    """Агент, который играет, используя RL стратегии (пока заглушки)."""
    def __init__(self, name: str):
        # Для простоты используем один LearningStrategy класс, но с разными role_name
        self.name = name
        self.strategies = {
            "MAFIA": LearningStrategy("MAFIA"),
            "CITIZEN": LearningStrategy("CITIZEN"),
             # ... и так далее
        }
    
    def _get_strategy(self, role: str) -> RoleStrategy:
        if role not in self.strategies:
            self.strategies[role] = LearningStrategy(role)
        return self.strategies[role]

# В будущем можно добавить LLMAgent, HeuristicAgent и т.д.
