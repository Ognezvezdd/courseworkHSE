
from abc import ABC, abstractmethod
from mafia.models import GameState, AgentAction

class RoleStrategy(ABC):
    """
    Базовый класс стратегии для конкретной роли.
    Определяет поведение агента, когда ему выпадает эта роль.
    """
    @abstractmethod
    def decide_action(self, game_state: GameState) -> AgentAction:
        pass
