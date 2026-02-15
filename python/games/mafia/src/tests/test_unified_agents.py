
import pytest
from games.mafia.src.models import GameState, PlayerState, AgentAction
from games.mafia.src.agents.unified_agents import RandomAgent, RLAgent
from games.mafia.src.agent_factory import AgentFactory

# Тесты для унифицированных агентов

def create_mock_game_state(phase="NIGHT_MAFIA", day=1, players=None):
    if players is None:
        players = [
            PlayerState(id=1, name="Alice", is_alive=True),
            PlayerState(id=2, name="Bob", is_alive=True),
            PlayerState(id=3, name="Charlie", is_alive=True)
        ]
    return GameState(phase=phase, day=day, players=players, chat_history=[], known_roles={})

def test_random_agent_all_roles():
    """Проверяем, что RandomAgent может играть за любую роль"""
    agent = RandomAgent("TestPlayer")
    state = create_mock_game_state()
    
    # Mafia
    action = agent.decide_action(state, "MAFIA")
    assert action.action_type == "MAFIA_KILL" or action.action_type == "PASS" # Заглушка может вернуть PASS если нет целей
    
    # Sheriff
    state.phase = "NIGHT_SHERIFF"
    action = agent.decide_action(state, "SHERIFF")
    assert action.action_type == "SHERIFF_CHECK"
    
     # Doctor
    state.phase = "NIGHT_DOCTOR"
    action = agent.decide_action(state, "DOCTOR")
    assert action.action_type == "DOCTOR_HEAL"
    
    # Citizen
    state.phase = "DAY_VOTING"
    action = agent.decide_action(state, "CITIZEN")
    assert action.action_type == "VOTE_KILL"

def test_rl_agent_initialization():
    """Проверяем инициализацию RL агента"""
    agent = RLAgent("RLPlayer")
    state = create_mock_game_state()
    
    # Проверка, что создается LearningStrategy
    action = agent.decide_action(state, "MAFIA")
    assert isinstance(action, AgentAction) # LearningStrategy возвращает AgentAction
    assert action.action_type is not None

def test_agent_factory_unified():
    """Проверка создания через фабрику"""
    agent = AgentFactory.get_agent("RANDOM", "RandomGuy")
    assert isinstance(agent, RandomAgent)
    
    agent = AgentFactory.get_agent("RL", "SmartGuy")
    assert isinstance(agent, RLAgent)
    
    # Неизвестный тип должен быть Random
    agent = AgentFactory.get_agent("UNKNOWN_TYPE", "WhoIsThis")
    assert isinstance(agent, RandomAgent)
