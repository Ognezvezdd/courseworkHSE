# Telegram Bot Client (C++)

Клиентская часть проекта, реализованная на C++. Отвечает за взаимодействие с пользователем через Telegram, управление ставками и запуск игр.

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

Перед запуском необходимо создать или отредактировать файл `config.json` в директории `cpp/` (или там, где запускается бинарный файл).

Пример `config.json`:

```json
{
    "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
    "python_path": "/usr/bin/python3",
    "game_script_path": "/absolute/path/to/courseworkHSE/python/run_game.py"
}
```

- **bot_token**: Токен вашего бота, полученный от @BotFather.
- **python_path**: Путь к интерпретатору Python (версия 3.10+).
- **game_script_path**: Абсолютный путь к скрипту `run_game.py` в папке `python`.

## Запуск

После успешной сборки и настройки:

```bash
./tg_bot
```

## Структура кода

- **`TgBot/`**: Логика бота.
  - `main.cpp`: Точка входа.
  - `bot.cpp/hpp`: Класс `TelegramBot`, обработка сообщений и Long Polling.
  - `keyboard.cpp/hpp`: Генерация клавиатур для меню.
- **`src/`**: Вспомогательные модули.
  - `game_manager.cpp/hpp`: Класс для запуска Python-скриптов и парсинга результатов.
  - `user_state.hpp`: Структура состояния пользователя.
  - `config.cpp/hpp`: Загрузка конфигурации.
