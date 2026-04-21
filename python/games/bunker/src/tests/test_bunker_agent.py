import pytest
import os
from ..models import BunkerState, PlayerState, ChatMessage
from ..agents.llm_gemma3_agent import BunkerLLMAgent

def create_mock_bunker_state(phase="DISCUSSION", my_id=1):
    players = [
        PlayerState(
            player_id=1, 
            player_name="Player_1", 
            profession="Doctor", 
            survival_score=80, 
            utility_score=90,
            is_alive=True
        ),
        PlayerState(
            player_id=2, 
            player_name="Player_2", 
            profession="Artist", 
            survival_score=30, 
            utility_score=20,
            is_alive=True
        ),
        PlayerState(
            player_id=3, 
            player_name="Player_3", 
            profession="Engineer", 
            survival_score=60, 
            utility_score=70,
            is_alive=False # Умер
        )
    ]
    return BunkerState(
        phase=phase,
        players=players,
        chat_history=[
            ChatMessage(player_id=2, player_name="Player_2", text="Я художник, я полезен для души!")
        ],
        my_player_id=my_id,
        known_info=["В бункере мало еды"]
    )

def test_bunker_build_prompt():
    agent = BunkerLLMAgent("Player_1", model="gemma3", personality="rational")
    state = create_mock_bunker_state()
    prompt = agent._build_prompt(state)
    
    # Проверяем наличие ключевых полей
    assert "Ты: Player_1" in prompt
    assert "Doctor" in prompt
    assert "Player_2" in prompt
    assert "80" in prompt # survival_score
    assert "90" in prompt # utility_score
    assert "Я художник" in prompt # История чата
    assert "В бункере мало еды" in prompt # Знания

def test_bunker_parse_action():
    agent = BunkerLLMAgent("Player_1")
    state = create_mock_bunker_state(phase="VOTING")
    
    # Валидный JSON
    text = '```json\n{"action_type": "VOTE_EXILE", "target_id": 2, "text_message": "Он бесполезен"}\n```'
    action = agent._parse(text, state)
    assert action.action_type == "VOTE_EXILE"
    assert action.target_id == 2
    
    # Попытка проголосовать за себя
    text_self = '{"action_type": "VOTE_EXILE", "target_id": 1}'
    action_self = agent._parse(text_self, state)
    assert action_self.action_type == "VOTE_EXILE" # Теперь возвращает случайного другого живого
    assert action_self.target_id == 2

    # Попытка проголосовать за мертвого
    text_dead = '{"action_type": "VOTE_EXILE", "target_id": 3}'
    action_dead = agent._parse(text_dead, state)
    assert action_dead.action_type == "VOTE_EXILE"
    assert action_dead.target_id == 2

def test_bunker_fallback():
    agent = BunkerLLMAgent("Player_1")
    state = create_mock_bunker_state(phase="VOTING")
    
    # Мусорный ответ
    action = agent._parse("Я не знаю что делать", state)
    assert action.action_type == "VOTE_EXILE" # Авто-выбор случайного живого
    assert action.target_id == 2
