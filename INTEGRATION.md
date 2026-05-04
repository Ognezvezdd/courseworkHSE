# Интеграция Python API и C++ Telegram Bot

Этот файл описывает текущее состояние интеграции между Python FastAPI и C++ Telegram-ботом.

## Текущая схема

```text
Telegram User
     |
     v
C++ Telegram Bot
     |
     | HTTP/JSON
     v
Python FastAPI
```

## Разделение ответственности

### Python

- `python/main_service.py` — основная точка входа FastAPI.
- `python/games/tictactoe/src/api.py` — API и движок Tic-Tac-Toe.
- `python/games/mafia/src/api.py` — endpoints для действий и сообщений Mafia-агентов.
- `python/games/bunker/src/api.py` — endpoints для действий и сообщений Bunker-агентов.
- `python/games/stats/api.py` — endpoints статистики.
- `python/games/common/llm_client.py` — общий клиент Ollama/OpenAI.
- `python/prompts/` — промпты для LLM-агентов.

### C++

- `cpp/TgBot/` — Telegram UI, меню и обработка команд.
- `cpp/src/game_manager.*` — координация запусков игр и запросов к API.
- `cpp/src/http_client.*` — HTTP-клиент на libcurl.
- `cpp/src/mafia_game.*` — текущий движок Mafia.
- `cpp/src/bunker_game.*` — текущий движок Bunker.
- `cpp/src/mafia_agent_proxy.*` и `cpp/src/bunker_agent_proxy.*` — прокси к Python API для действий агентов.

## Как запускается каждая игра

| Игра | Где выполняется игровой цикл | Как используются Python endpoints |
| --- | --- | --- |
| Tic-Tac-Toe | Python | `POST /game/play`, `POST /train` |
| Mafia | C++ | `POST /mafia/agent_action`, `POST /mafia/agent_chat` |
| Bunker | C++ | `POST /bunker/agent_action`, `POST /bunker/agent_chat` |

## Docker Compose

`docker-compose.yml` поднимает два сервиса:

- `python-api` — FastAPI на порту `8000`.
- `telegram-bot` — C++ бот, который ждет healthcheck Python API.

Конфигурация бота монтируется из `./cpp/config.json` в `/app/config.json`.

Для Docker в `cpp/config.json` должен быть:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://python-api:8000"
}
```

## Локальный запуск

### Terminal 1: Python API

```bash
cd python
python3 -m pip install -r common/requirements.txt
python3 -m uvicorn main_service:app --reload --host 0.0.0.0 --port 8000
```

### Terminal 2: C++ Bot

```bash
cd cpp
mkdir -p build
cd build
cmake ..
make
./tg_bot
```

Для локального запуска в `cpp/config.json` используйте:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://localhost:8000"
}
```

## Проверка интеграции

### Python API работает

```bash
curl http://localhost:8000/
curl http://localhost:8000/agents
curl http://localhost:8000/docs
```

### Бот подключается к API

```bash
docker-compose logs telegram-bot
```

В логах должна быть успешная проверка API.

### Tic-Tac-Toe endpoint

```bash
curl -X POST http://localhost:8000/game/play \
  -H "Content-Type: application/json" \
  -d '{"agent_x":"heuristic","agent_o":"random","seed":42}'
```

### Stats endpoint

```bash
curl http://localhost:8000/stats/report
```

## LLM

Для Ollama-агентов должен быть доступен Ollama:

```bash
OLLAMA_HOST=0.0.0.0 ollama serve
ollama pull gemma3
```

В Docker Python API обращается к Ollama по `http://host.docker.internal:11434/api/generate`. При локальном запуске общий LLM-клиент автоматически пробует `http://localhost:11434/api/generate`, если основной endpoint недоступен.

## Тестирование

Тесты находятся внутри пакетов игр:

```bash
cd python
python3 -m pytest -q
python3 -m pytest games/tictactoe/src/tests -v
python3 -m pytest games/mafia/src/tests -v
python3 -m pytest games/bunker/src/tests -v
```

Ручная интеграционная проверка Mafia API:

```bash
python3 games/mafia/src/tests/manual_mafia_llm_api_check.py
```

## Секреты

- `cpp/config.json.example` — шаблон, его можно хранить в Git.
- `cpp/config.json` — локальный файл с токеном, его нельзя коммитить.
- Если настоящий Telegram token попал в Git, его нужно перевыпустить у `@BotFather`.
