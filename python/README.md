# Python AI Platform Core

This directory contains the core simulation engines and AI agents for the project.

## Structure

- **`games/`**: Implementation of specific games.
  - **`tictactoe/`**: Tic-Tac-Toe 5x5 implementation.
  - **`mafia/`**: Mafia game logic and AI agents.
- **`agents/`**: LLM-агенты (Gemma 3) и базовые классы.
- **`visualization/`**: Shared tools for game state visualization.
- **`main_service.py`**: The entry point for the FastAPI server that combines all games.

## Local Development

### Running the API Server

```bash
cd python
pip install -r requirements.txt
python3 main_service.py
```

### LLM Requirements
Для использования `llm` агентов необходимо запустить Ollama:
```bash
OLLAMA_HOST=0.0.0.0 ollama serve
```
Модель по умолчанию: `gemma3`.

Or using uvicorn:
```bash
uvicorn main_service:app --reload --host 0.0.0.0 --port 8000
```

The API will be available at `http://localhost:8000/docs`.

### Running Tests

```bash
# Run all tests
pytest tests/

# Run llm tests
pytest games/mafia/src/tests/test_llm_agent.py

# Run mafia tests
pytest games/mafia/src/tests/

# Run tictactoe tests
pytest games/tictactoe/src/tests/
```

## Games Documentation

- [📖 Mafia README](./games/mafia/src/README.md)
- [📖 Tic-Tac-Toe README](./games/tictactoe/src/README.md)
