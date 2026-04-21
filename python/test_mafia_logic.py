import json
import os
import sys

# Добавляем корень проекта в путь
sys.path.append('/app')

from games.mafia.src.models import GameState, PlayerState, ChatMessage, VotingRecord
from games.mafia.src.agents.llm_gemma3_agent import LLMAgent

def test_anonymization():
    agent = LLMAgent(name="llm_gemma3", model="gemma3")
    
    # Имитируем состояние игры
    
    # Анонимизированный чат (как его готовит api.py)
    chat = [
        ChatMessage(player_id=1, player_name="Player_1", player_role="", text="Привет всем! Я мирный.", is_public=True),
        ChatMessage(player_id=2, player_name="Player_2", player_role="", text="Не верю тебе, Player_1.", is_public=True)
    ]
    
    # История голосований
    voting = [
        VotingRecord(day=1, votes={"Player_1": 1, "Player_3": 2}, exiled="Player_3")
    ]
    
    game_state = GameState(
        phase="DAY_DISCUSSION",
        day=2,
        players=[
            PlayerState(id=1, name="Player_1", role="MAFIA", is_alive=True),
            PlayerState(id=2, name="Player_2", role="CITIZEN", is_alive=True),
            PlayerState(id=3, name="Player_3", role="MAFIA", is_alive=False),
            PlayerState(id=4, name="Player_4", role="SHERIFF", is_alive=False)
        ],
        chat_history=chat,
        my_id=1,
        my_role="MAFIA",
        voting_history=voting,
        eliminated_players=["День 1: изгнан Player_3"],
        mafia_team_ids=[1, 3]
    )
    
    prompt = agent.build_prompt(game_state, "MAFIA")
    
    print("--- ТЕСТ ПРОМПТА ---")
    print(prompt)
    
    # Проверки
    assert "llm_gemma3" not in prompt, "Настоящее имя агента не должно быть в промпте"
    assert "citizen_cautious" not in prompt, "Настоящие имена игроков не должны быть в промпте"
    assert "mafia_conservative" not in prompt, "Настоящие имена игроков не должны быть в промпте"
    assert "Player_1" in prompt, "Алиасы должны присутствовать"
    assert "Player_2" in prompt, "Алиасы должны присутствовать"
    assert "Твои союзники в мафии: Player_3" in prompt, "Союзники должны быть видны мафии"
    print("\n✅ Тест анонимизации пройден успешно!")

if __name__ == "__main__":
    try:
        test_anonymization()
    except Exception as e:
        print(f"\n❌ Ошибка: {e}")
        import traceback
        traceback.print_exc()
