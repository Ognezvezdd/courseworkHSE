from .models import GameState, AgentAction
from .agents.unified_agents import RandomAgent
from .agents.llm_gemma3_agent import LLMAgent

class AgentFactory:
    AGENT_CLASS_MAP = {
        "RANDOM": RandomAgent,
        "LLM": LLMAgent,
    }

    @staticmethod
    def get_agent(agent_type: str, name: str) -> 'BaseAgent':
        agent_cls = AgentFactory.AGENT_CLASS_MAP.get(agent_type.upper(), RandomAgent)
        return agent_cls(name)

    @staticmethod
    def get_action_for_player(
        game_state: GameState,
        my_role: str,
        my_name: str,
        agent_type: str = "RANDOM",
    ) -> AgentAction:
        agent = AgentFactory.get_agent(agent_type, my_name)
        return agent.decide_action(game_state, my_role)
