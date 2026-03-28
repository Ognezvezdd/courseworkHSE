import pytest
from ..models import GameState, PlayerState
from ..agents.llm_gemma3_agent import LLMAgent
from ..agent_factory import AgentFactory

def create_mock_game_state(phase="NIGHT_MAFIA", day=1):
    players = [
        PlayerState(id=1, name="Alice", is_alive=True),
        PlayerState(id=2, name="Bob", is_alive=True),
        PlayerState(id=3, name="Charlie", is_alive=False)
    ]
    return GameState(phase=phase, day=day, players=players, chat_history=[], known_roles={})

def test_llm_agent_factory_creation():
    agent = AgentFactory.get_agent("LLM", "TestLLM")
    assert isinstance(agent, LLMAgent)
    assert agent.name == "TestLLM"

def test_llm_build_prompt():
    agent = LLMAgent("AgentLLM")
    state = create_mock_game_state()
    prompt = agent.build_prompt(state, "MAFIA")
    
    assert "Your name is 'AgentLLM'" in prompt
    assert "Your game role is 'MAFIA'" in prompt
    assert "[ID:1] Alice" in prompt
    assert "[ID:3] Charlie" not in prompt # Should only show alive players

def test_llm_parse_action():
    agent = LLMAgent("AgentLLM")
    state = create_mock_game_state()
    
    # Test valid JSON within markdown
    text = "Here is my move:\n```json\n{\"action_type\": \"MAFIA_KILL\", \"target_id\": 2}\n```"
    action = agent.parse_action(text, state)
    assert action.action_type == "MAFIA_KILL"
    assert action.target_id == 2
    
    # Test garbage string
    text = "I think I'll just pass today."
    action = agent.parse_action(text, state)
    assert action.action_type == "PASS"

    # Test invalid action_type fallback
    text = "{\"action_type\": \"INVALID_STUFF\", \"target_id\": 1}"
    action = agent.parse_action(text, state)
    assert action.action_type == "PASS" # Validates against valid_actions
