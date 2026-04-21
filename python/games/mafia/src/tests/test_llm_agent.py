import pytest
from ..models import GameState, PlayerState
from ..agents.llm_gemma3_agent import LLMAgent
from ..agent_factory import AgentFactory

def create_mock_game_state(phase="NIGHT_MAFIA", day=1, my_id=1):
    players = [
        PlayerState(id=1, name="Alice", is_alive=True),
        PlayerState(id=2, name="Bob", is_alive=True),
        PlayerState(id=3, name="Charlie", is_alive=False)
    ]
    return GameState(
        phase=phase, 
        day=day, 
        players=players, 
        chat_history=[], 
        known_roles={},
        my_id=my_id
    )

def test_llm_agent_factory_creation():
    agent = AgentFactory.get_agent("LLM", "TestLLM")
    assert isinstance(agent, LLMAgent)
    assert agent.name == "TestLLM"

def test_llm_build_prompt():
    agent = LLMAgent("AgentLLM")
    state = create_mock_game_state(my_id=1)
    prompt = agent.build_prompt(state, "MAFIA")
    
    # Теперь промпт на русском и с алиасами
    assert "Ты: Player_1" in prompt
    assert "Роль: МАФИЯ" in prompt
    assert "[ID:1] Player_1" in prompt
    assert "[ID:2] Player_2" in prompt
    assert "Alice" not in prompt # Имена должны быть анонимизированы
    assert "Charlie" not in prompt # Мертвые не должны отображаться в списке живых

def test_llm_parse_action():
    agent = LLMAgent("AgentLLM")
    state = create_mock_game_state()
    role = "MAFIA"
    
    # Test valid JSON within markdown
    text = "Here is my move:\n```json\n{\"action_type\": \"MAFIA_KILL\", \"target_id\": 2}\n```"
    action = agent.parse_action(text, state, role)
    assert action.action_type == "MAFIA_KILL"
    assert action.target_id == 2
    
    # Test garbage string -> CHAT_MESSAGE fallback (for discussion phase)
    state_disc = create_mock_game_state(phase="DAY_DISCUSSION")
    text = "I think I'll just pass today."
    action = agent.parse_action(text, state_disc, role)
    # В фазе обсуждения fallback вернет CHAT_MESSAGE, если не нашел JSON
    assert action.action_type in ["CHAT_MESSAGE", "PASS"]

    # Test invalid action_type fallback
    text = "{\"action_type\": \"INVALID_STUFF\", \"target_id\": 1}"
    action = agent.parse_action(text, state, role)
    assert action.action_type == "PASS" # Validates against valid_actions
