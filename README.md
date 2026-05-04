# Курсовая работа ПИ 2 курс: AI Agents Platform

**Платформа симуляции конкурентного взаимодействия AI-агентов**  
*(Competitive Multi-Agent Simulation Platform)*

### Авторы

* Капогузов М. Е.
* Дудкина А. Э.

---

## О проекте

Проект моделирует соревнование AI-агентов в настольных играх: **Крестики-нолики 5x5**, **Мафия** и **Бункер**. Основной фокус — сравнение поведения разных типов агентов, включая эвристические стратегии, Q-Learning и LLM-агентов через Ollama/OpenAI.

Текущая архитектура состоит из двух частей:

1. **Python FastAPI service** — REST API, игровой движок для Tic-Tac-Toe, агенты, LLM-прокси, промпты, статистика и визуализация Tic-Tac-Toe.
2. **C++ Telegram bot** — пользовательский интерфейс в Telegram, управление играми, состояние пользователей, а также текущие игровые движки Mafia и Bunker. Для решений LLM-агентов C++ обращается в Python API.

Важно: в текущей версии Mafia и Bunker симулируются в C++-части, а Python API используется для получения действий и реплик агентов.

---

## Архитектура

```text
Telegram User
     |
     v
C++ Telegram Bot
     |
     | HTTP/JSON
     v
Python FastAPI
     |
     +-- Tic-Tac-Toe engine and agents
     +-- Mafia/Bunker agent decision endpoints
     +-- Stats endpoints
     +-- Ollama/OpenAI calls for LLM agents
```

### Основные сценарии

- Tic-Tac-Toe запускается через `POST /game/play` в Python API.
- Mafia запускается в C++ (`cpp/src/mafia_game.*`), а действия агентов запрашиваются через `/mafia/agent_action` и `/mafia/agent_chat`.
- Bunker запускается в C++ (`cpp/src/bunker_game.*`), а действия агентов запрашиваются через `/bunker/agent_action` и `/bunker/agent_chat`.
- Статистика записывается через `/stats/record/*` и читается через `/stats/report`.

---

## Структура репозитория

| Путь | Назначение |
| --- | --- |
| `python/main_service.py` | Основная точка входа FastAPI |
| `python/common/requirements.txt` | Python-зависимости |
| `python/games/tictactoe/` | Tic-Tac-Toe 5x5: движок, агенты, API, тесты |
| `python/games/mafia/` | Python-агенты и API для Mafia |
| `python/games/bunker/` | Python-агенты и API для Bunker |
| `python/prompts/` | Промпты для LLM-агентов |
| `python/games/common/stats_manager.py` | JSON-статистика бенчмарка |
| `cpp/TgBot/` | Telegram-интерфейс |
| `cpp/src/` | C++ игровые движки, HTTP-клиент, менеджер игр |
| `docker-compose.yml` | Запуск Python API и Telegram-бота |
| `Makefile`, `deploy.sh` | Удобные команды запуска |

---

## Быстрый запуск через Docker Compose

1. Создайте локальный конфиг бота:

```bash
cp cpp/config.json.example cpp/config.json
```

2. Вставьте токен Telegram-бота от `@BotFather` в `cpp/config.json`.

Пример для Docker:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://python-api:8000"
}
```

3. Запустите платформу:

```bash
docker-compose up --build
```

Swagger UI будет доступен на `http://localhost:8000/docs`.

Также можно использовать:

```bash
make up
# или
./deploy.sh start
```

---

## Локальный запуск для разработки

### Python API

```bash
cd python
python3 -m pip install -r common/requirements.txt
python3 main_service.py
```

Альтернативно:

```bash
cd python
python3 -m uvicorn main_service:app --reload --host 0.0.0.0 --port 8000
```

### C++ Telegram Bot

```bash
cd cpp
mkdir -p build
cd build
cmake ..
make
./tg_bot
```

Для локального запуска в `cpp/config.json` укажите:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://localhost:8000"
}
```

---

## LLM-зависимости

Для локальных LLM-агентов нужен запущенный Ollama:

```bash
OLLAMA_HOST=0.0.0.0 ollama serve
```

Минимальная модель для проверки:

```bash
ollama pull gemma3
```

Модели, которые используются в меню/бенчмарке:

```bash
ollama pull gemma3
ollama pull llama3.2:1b
ollama pull llama3.2:3b
ollama pull phi3:mini
ollama pull phi4-mini
ollama pull qwen2.5:1.5b
```

OpenAI-модели используются только если пользователь добавил API key через настройки Telegram-бота или если задан `OPENAI_API_KEY`.

---

## API

### Общие endpoints

- `GET /` — health check.
- `GET /agents` — агенты для Tic-Tac-Toe.
- `POST /game/play` — запуск Tic-Tac-Toe.
- `POST /train` — обучение Q-Learning агента для Tic-Tac-Toe.

Пример `POST /game/play`:

```json
{
  "agent_x": "heuristic",
  "agent_o": "random",
  "seed": 42
}
```

### Mafia

- `POST /mafia/agent_action` — действие агента.
- `POST /mafia/agent_chat` — реплика агента для обсуждения.

### Bunker

- `POST /bunker/agent_action` — действие агента.
- `POST /bunker/agent_chat` — реплика агента для обсуждения.

### Stats

- `POST /stats/record/tictactoe`
- `POST /stats/record/mafia`
- `POST /stats/record/bunker`
- `GET /stats/report`

---

## Тестирование

Тесты расположены внутри пакетов игр и запускаются из директории `python`:

```bash
cd python
python3 -m pytest -q
python3 -m pytest games/tictactoe/src/tests -v
python3 -m pytest games/mafia/src/tests -v
python3 -m pytest games/bunker/src/tests -v
```

Интеграционную проверку Mafia API можно запустить при поднятом FastAPI:

```bash
cd python
python3 games/mafia/src/tests/manual_mafia_llm_api_check.py
```

---

## Конфигурация и секреты

- `cpp/config.json.example` хранится в репозитории как шаблон.
- `cpp/config.json` предназначен для локальных секретов и не должен попадать в Git.
- Если реальный Telegram token когда-либо был закоммичен, его нужно перевыпустить у `@BotFather`.

---

## Возможности платформы

- Tic-Tac-Toe 5x5: Python engine, агенты, API, визуализация.
- Mafia: C++ engine, Python endpoints для действий и чата агентов.
- Bunker: C++ engine, Python endpoints для действий и чата агентов.
- Telegram bot на C++.
- FastAPI gateway.
- Интеграция Ollama для локальных LLM.
- Интеграция OpenAI через пользовательский API key.
- JSON-статистика бенчмарка с валидацией входных данных.
- Docker Compose для совместного запуска API и Telegram-бота.

---

## Troubleshooting

### API не отвечает

```bash
curl http://localhost:8000/
docker-compose logs python-api
```

### Бот не видит API

- В Docker `api_url` должен быть `http://python-api:8000`.
- При локальном запуске `api_url` должен быть `http://localhost:8000`.

### LLM-агенты не отвечают

```bash
ollama list
curl http://localhost:11434/api/tags
```

Проверьте, что Ollama запущен и нужная модель скачана.

### C++ бот не собирается

macOS:

```bash
brew install cmake jsoncpp libcurl
```

Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y cmake libcurl4-openssl-dev libjsoncpp-dev build-essential
```

---

## Лицензия

Проект создан в рамках курсовой работы НИУ ВШЭ.
