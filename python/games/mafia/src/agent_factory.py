
from .models import GameState, AgentAction, PlayerState
from .agents.unified_agents import RandomAgent, RLAgent
from typing import Dict, Type

class AgentFactory:
    """
    Фабрика для создания унифицированных агентов.
    Теперь агенты создаются по типу интеллекта (Random, RL), а не по роли.
    Роль задается в процессе игры.
    """
    
    AGENT_CLASS_MAP = {
        "RANDOM": RandomAgent,
        "RL": RLAgent, 
        # "LLM": LLMAgent, # Заглушка
    }

    @staticmethod
    def get_agent(agent_type: str, name: str) -> 'BaseAgent':
        agent_cls = AgentFactory.AGENT_CLASS_MAP.get(agent_type.upper(), RandomAgent)
        return agent_cls(name)

    @staticmethod
    def get_action_for_player(game_state: GameState, my_role: str, my_name: str, agent_type: str = "RANDOM") -> AgentAction:
        """
        Управляет вызовом агента.
        Теперь мы передаем agent_type, чтобы выбрать нужного *игрока* (Random, AI).
        """
        agent = AgentFactory.get_agent(agent_type, my_name)
        
        # Передаем роль внутрь агента
        return agent.decide_action(game_state, my_role)
