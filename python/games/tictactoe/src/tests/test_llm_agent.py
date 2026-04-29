import pytest
from games.tictactoe.src.agents.llm_gemma3_agent import LLMAgent

def test_llm_build_prompt():
    agent = LLMAgent(name="TestLLM")
    board = [
        [".", "X", "."],
        [".", "O", "."],
        [".", ".", "."]
    ]
    prompt = agent.build_prompt(board, "X")
    
    assert "Your symbol is 'X'" in prompt
    assert "Row 0: . X ." in prompt
    assert "[0, 0]" in prompt
    assert "[0, 1]" not in prompt # Cannot move on X

def test_llm_parse_action(monkeypatch):
    agent = LLMAgent(name="TestLLM")
    board = [
        [".", "X", "."],
        [".", "O", "."],
        [".", ".", "."]
    ]
    
    class MockResponse:
        def __init__(self, json_data):
            self.json_data = json_data
        
        def json(self):
            return self.json_data
            
        def raise_for_status(self):
            pass

    def mock_post(*args, **kwargs):
        return MockResponse({"response": "```json\n{\"row\": 0, \"col\": 2}\n```"})

    monkeypatch.setattr("requests.post", mock_post)
    
    action = agent.select_action(board, "X")
    assert action == (0, 2)
    
    def mock_post_invalid(*args, **kwargs):
        return MockResponse({"response": "I cannot do that."})
        
    monkeypatch.setattr("requests.post", mock_post_invalid)
    action2 = agent.select_action(board, "X")
    valid_moves = agent._get_valid_moves(board)
    assert action2 in valid_moves
    assert action2 == (0, 0) # Fallbacks to first valid move
