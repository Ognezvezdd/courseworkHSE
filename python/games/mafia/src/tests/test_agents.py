
import pytest
from mafia.models import GameState, PlayerState
from mafia.agent_factory import AgentFactory

# Простые тесты для агентов мафии

def create_mock_game_state(phase="NIGHT_MAFIA", day=1, players=None):
    if players is None:
        players = [
            PlayerState(id=1, name="Alice", is_alive=True),
            PlayerState(id=2, name="Bob", is_alive=True),
            PlayerState(id=3, name="Charlie", is_alive=True)
        ]
    return GameState(phase=phase, day=day, players=players, chat_history=[], known_roles={})

def test_mafia_night_action():
    state = create_mock_game_state(phase="NIGHT_MAFIA")
    agent = AgentFactory.get_agent("MAFIA", "MafiaBoss")
    action = agent.decide_action(state)
    
    assert action.action_type == "MAFIA_KILL"
    assert action.target_id in [1, 2, 3]

def test_sheriff_night_action():
    state = create_mock_game_state(phase="NIGHT_SHERIFF")
    agent = AgentFactory.get_agent("SHERIFF", "Detective")
    action = agent.decide_action(state)
    
    assert action.action_type == "SHERIFF_CHECK"
    assert action.target_id in [1, 2, 3]

def test_citizen_day_vote():
    state = create_mock_game_state(phase="DAY_VOTING")
    agent = AgentFactory.get_agent("CITIZEN", "Villager")
    action = agent.decide_action(state)
    
    assert action.action_type == "VOTE_KILL"
    assert action.target_id in [1, 2, 3]

def test_doctor_night_heal():
    state = create_mock_game_state(phase="NIGHT_DOCTOR")
    agent = AgentFactory.get_agent("DOCTOR", "Medic")
    action = agent.decide_action(state)
    
    assert action.action_type == "DOCTOR_HEAL"
    assert action.target_id in [1, 2, 3]

def test_unknown_phase():
    state = create_mock_game_state(phase="UNKNOWN_PHASE")
    agent = AgentFactory.get_agent("CITIZEN", "Villager")
    action = agent.decide_action(state)
    
    # Должен ничего не делать
    assert action.action_type == "PASS"
