# Telegram Bot Client (C++)

Клиентская часть проекта, реализованная на C++. Отвечает за взаимодействие с пользователем через Telegram, управление ставками и запуск игр **через REST API**.

## Архитектура взаимодействия

```
Telegram User  ◀──▶  C++ Bot  ──HTTP API──▶  Python FastAPI  ──▶  AI Agents
                       │                          │
                       │                          │
                  libcurl + JSON            Game Engine + ML
```

**Ключевые компоненты:**
- `HttpClient`: HTTP клиент на основе libcurl для взаимодействия с API
- `GameManager`: Менеджер игр, отправляющий запросы к Python API
- `TelegramBot`: Обработчик Telegram команд и сообщений

## Возможности

- **Интерфейс**: Удобное меню с кнопками для выбора игры, агента и ставки.
- **Экономика**: Система ставок и баланса пользователей.
- **Агенты**: Поддержка различных стратегий (Random, Heuristic, Q-Learning).
- **Интеграция**: Запуск Python-скриптов для проведения матчей.

## Требования

Для сборки проекта необходимы:

- **Приложения**: `cmake`, `make`, `g++` (или `clang++`) с поддержкой C++17.
- **Библиотеки**:
  - `libcurl` (для сетевых запросов)
  - `jsoncpp` (для работы с JSON)

### Установка зависимостей (macOS)

```bash
brew install cmake jsoncpp libcurl
```

### Установка зависимостей (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install cmake libcurl4-openssl-dev libjsoncpp-dev build-essential
```

## Сборка

Проект использует систему сборки CMake.

```bash
# Перейдите в директорию cpp
cd cpp

# Создайте директорию для сборки
mkdir build && cd build

# Сгенерируйте файлы сборки
cmake ..

# Соберите проект
make
```

## Конфигурация

Перед запуском необходимо создать или отредактировать файл `config.json` в директории `cpp/`.

Пример `config.json`:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://localhost:8000"
}
```

Для Docker:
```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "api_url": "http://python-api:8000"
}
```

**Параметры:**
- **bot_token**: Токен вашего бота, полученный от @BotFather в Telegram.
- **api_url**: URL Python FastAPI сервиса.
  - Локально: `http://localhost:8000`
  - Docker: `http://python-api:8000`

## Запуск

После успешной сборки и настройки:

```bash
./tg_bot
```

## Структура кода

- **`TgBot/`**: Логика бота.
  - `main.cpp`: Точка входа, инициализация и проверка API.
  - `bot.cpp/hpp`: Класс `TelegramBot`, обработка сообщений и Long Polling.
  - `keyboard.cpp/hpp`: Генерация клавиатур для меню.
- **`src/`**: Вспомогательные модули.
  - `http_client.cpp/hpp`: HTTP клиент на основе libcurl для API запросов.
  - `game_manager.cpp/hpp`: Менеджер игр, взаимодействие с Python API.
  - `user_state.hpp`: Структура состояния пользователя.
  - `config.cpp/hpp`: Загрузка конфигурации.

## Зависимости между модулями

```
main.cpp
  ├─▶ config.cpp        (загрузка конфигурации)
  ├─▶ game_manager.cpp  (взаимодействие с API)
  │     └─▶ http_client.cpp  (HTTP запросы)
  └─▶ bot.cpp           (Telegram интерфейс)
        └─▶ keyboard.cpp     (UI элементы)
```

