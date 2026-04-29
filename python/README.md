# Python AI Platform Service

This directory contains the FastAPI service, Tic-Tac-Toe engine, Python agent logic, prompts, visualization helpers, and benchmark stats storage.

## Structure

- **`games/`**: Game-specific Python modules.
  - **`tictactoe/`**: Tic-Tac-Toe 5x5 engine, agents, API, and tests.
  - **`mafia/`**: Agent decision API, models, strategies, and tests for the C++ Mafia engine.
  - **`bunker/`**: Agent decision API, models, prompts, logging, and tests for the C++ Bunker engine.
- **`games/common/`**: Shared LLM client and JSON stats manager.
- **`prompts/`**: Prompt templates for LLM agents.
- **`visualization/`**: Shared tools for game state visualization.
- **`main_service.py`**: The FastAPI entry point that combines all routers.

## Local Development

### Running the API Server

```bash
cd python
python3 -m pip install -r common/requirements.txt
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
python3 -m uvicorn main_service:app --reload --host 0.0.0.0 --port 8000
```

The API will be available at `http://localhost:8000/docs`.

### Running Tests

```bash
# Run Tic-Tac-Toe tests
python3 -m pytest games/tictactoe/src/tests -v

# Run mafia tests
python3 -m pytest games/mafia/src/tests -v

# Run bunker tests
python3 -m pytest games/bunker/src/tests -v
```

Current note: the complete test collection still needs cleanup. Some tests depend on package import context, and `llm/tests/test_ollama_basic.py` calls Ollama during import, so run it only when Ollama is available.

```bash
python3 -m pytest llm/tests/test_ollama_basic.py -v
```

## Games Documentation

- [📖 Mafia README](./games/mafia/src/README.md)
- [📖 Tic-Tac-Toe README](./games/tictactoe/src/README.md)
