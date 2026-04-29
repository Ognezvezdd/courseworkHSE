# Telegram Bot Client (C++)

Клиентская часть проекта, реализованная на C++. Она отвечает за Telegram-интерфейс, меню, состояние пользователей, запуск игр и обращение к Python FastAPI.

В текущей версии C++ содержит игровые движки **Mafia** и **Bunker**. Python API используется для Tic-Tac-Toe, статистики и решений Python/LLM-агентов.

## Архитектура взаимодействия

```text
Telegram User  <-->  C++ Bot  --HTTP/JSON-->  Python FastAPI
                       |                          |
                       |                          |
                 Mafia/Bunker engines       Tic-Tac-Toe, stats,
                                            LLM agent decisions
```

## Ключевые компоненты

- `HttpClient`: HTTP-клиент на основе libcurl.
- `GameManager`: запускает Tic-Tac-Toe через Python API, Mafia/Bunker через C++ движки и отправляет статистику в Python API.
- `TelegramBot`: обработчик команд, меню и сообщений Telegram.
- `MafiaAgentProxy`, `BunkerAgentProxy`: прокси к Python API для действий и реплик агентов.

## Возможности

- Меню Telegram для выбора игр и настроек.
- Tic-Tac-Toe 5x5 через Python API.
- Mafia и Bunker через C++ игровые циклы.
- LLM-агенты через Python API, Ollama и частично OpenAI.
- JSON-статистика бенчмарка через Python API.

## Требования

- `cmake`, `make`, `g++` или `clang++` с поддержкой C++17.
- `libcurl`.
- `jsoncpp`.

### macOS

```bash
brew install cmake jsoncpp libcurl
```

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y cmake libcurl4-openssl-dev libjsoncpp-dev build-essential
```

## Сборка

```bash
cd cpp
mkdir -p build
cd build
cmake ..
make
```

## Конфигурация

Перед запуском создайте `cpp/config.json` из шаблона:

```bash
cp cpp/config.json.example cpp/config.json
```

Локальный запуск:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://localhost:8000"
}
```

Docker:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://python-api:8000"
}
```

`cpp/config.json` содержит секреты и не должен попадать в Git.

## Запуск

Сначала запустите Python API, затем:

```bash
cd cpp/build
./tg_bot
```

## Структура кода

- `TgBot/main.cpp`: точка входа, загрузка конфига, проверка API.
- `TgBot/bot.cpp`, `TgBot/bot.hpp`: Telegram bot loop, сообщения и сценарии игр.
- `TgBot/keyboard.cpp`, `TgBot/keyboard.hpp`: генерация меню.
- `src/http_client.cpp`, `src/http_client.hpp`: HTTP-запросы.
- `src/game_manager.cpp`, `src/game_manager.hpp`: координация игр и статистики.
- `src/mafia_game.cpp`, `src/mafia_game.hpp`: C++ движок Mafia.
- `src/bunker_game.cpp`, `src/bunker_game.hpp`: C++ движок Bunker.
- `src/mafia_agent_proxy.cpp`, `src/bunker_agent_proxy.cpp`: запросы к Python API для агентов.
- `src/config.cpp`, `src/config.hpp`: загрузка `config.json` и `API_URL`.
- `src/user_state.hpp`: состояние пользователя Telegram.

## Зависимости между модулями

```text
main.cpp
  +-- config.cpp
  +-- game_manager.cpp
  |     +-- http_client.cpp
  |     +-- mafia_game.cpp
  |     +-- bunker_game.cpp
  |     +-- mafia_agent_proxy.cpp
  |     +-- bunker_agent_proxy.cpp
  +-- bot.cpp
        +-- keyboard.cpp
```
