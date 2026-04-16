# Курсовая работа ПИ 2 курс: AI Agents Platform

**Платформа симуляции конкурентного взаимодействия AI-агентов**  
*(Competitive Multi-Agent Simulation Platform)*

### Авторы: 
* Капогузов М. Е.
* Дудкина А. Э.

---

Это модульная платформа для разработки, тестирования и соревнований искусственных интеллектов (бенчмарк для LLM) в настольных играх (Крестики-нолики 5x5, Мафия и Бункер).

Система состоит из двух функциональных блоков:
1.  **Backend (Python)**: Ядро симуляции, реализация агентов (Random, Heuristic, RL, **LLM**) и REST API.
2.  **Client (C++)**: Telegram-бот интерфейс для управления симуляциями и наблюдения за матчами.

### Архитектура взаимодействия

```
┌─────────────────┐      HTTP API       ┌──────────────────┐
│   C++ Telegram  │ ◀──────────────────▶ │   Python FastAPI │
│      Bot        │  (JSON requests)     │   Game Engine    │
└─────────────────┘                      └──────────────────┘
        │                                         │
        │                                         │
        ▼                                         ▼
  Telegram Users                          AI Agents (Random,
 (Управление и                           Heuristic, Q-Learning,
  наблюдение)                            **Local & Cloud LLMs**)
```

**Преимущества микросервисной архитектуры:**
- ✅ **Изоляция компонентов**: Python и C++ работают независимо
- ✅ **Масштабируемость**: Легко добавлять новые боты или API инстансы
- ✅ **Простота развертывания**: Docker Compose для одной команды запуска
- ✅ **Тестируемость**: Каждый сервис можно тестировать отдельно

---

## Структура репозитория

| Директория | Описание | Документация |
|------------|----------|--------------|
| **`python/`** | Игровое ядро, агенты, API, тесты | [📖 Python README](./python/README.md) |
| **`cpp/`** | Telegram-бот, логика ставок | [📖 C++ README](./cpp/README.md) |
| **`docker-compose.yml`** | Оркестрация сервисов | - |

---

## Быстрый запуск

### Вариант 1: Docker Compose (Рекомендуется)

Самый простой способ запустить всю систему:

```bash
# 1. Клонируйте репозиторий
git clone <your-repo-url>
cd courseworkHSE

# 2. Настройте Telegram токен
# Скопируйте пример конфига и вставьте свой токен от @BotFather
cp cpp/config.json.example cpp/config.json
# Отредактируйте cpp/config.json, заменив YOUR_BOT_TOKEN_HERE на реальный токен

# 3. Запустите все сервисы
docker-compose up --build

# Готово! Бот доступен в Telegram

### Важное замечание по LLM (Gemma 3)
Для работы LLM-агентов необходимо, чтобы на хост-машине был запущен **Ollama** с моделью `gemma3`.
1. Установите Ollama с [ollama.com](https://ollama.com).
2. Запустите Ollama, разрешив внешние соединения (для Docker):
   ```bash
   OLLAMA_HOST=0.0.0.0 ollama serve
   ```
3. Скачайте модель:
   ```bash
   ollama run gemma3
   ```
```

**Архитектура в Docker:**
- `python-api` (порт 8000): FastAPI сервер с игровым движком
- `telegram-bot`: C++ бот, взаимодействующий с API
- Автоматическая настройка сети между контейнерами
- Health checks для проверки доступности API

### Вариант 2: Локальный запуск (для разработки)

#### 1. Python API

```bash
cd python
pip install -r requirements.txt

# Запуск API сервера
python3 main_service.py
# Или через uvicorn
uvicorn main_service:app --reload --host 0.0.0.0 --port 8000

# Swagger UI доступен на http://localhost:8000/docs
```

#### 2. C++ Telegram Bot

```bash
cd cpp

# Установка зависимостей (macOS)
brew install cmake jsoncpp libcurl

# Установка зависимостей (Ubuntu)
sudo apt-get install cmake libcurl4-openssl-dev libjsoncpp-dev build-essential

# Сборка
mkdir build && cd build
cmake ..
make

# Настройка config.json
# Убедитесь что api_url указывает на http://localhost:8000 (или другой адрес вашего API)

# Запуск
./tg_bot
```

---

## Технологический стек

### Backend (Python)
*   **Язык**: Python 3.11
*   **Веб-фреймворк**: FastAPI
*   **ASGI сервер**: Uvicorn
*   **Вычисления**: NumPy
*   **Визуализация**: Matplotlib
*   **Тестирование**: Pytest
*   **AI**: Q-Learning (Reinforcement Learning), Heuristic Algorithms, **LLM (Gemma 3 via Ollama API)**

### Client (C++)
*   **Язык**: C++17
*   **Сборка**: CMake
*   **HTTP клиент**: libcurl
*   **JSON парсинг**: JsonCpp
*   **Telegram Bot API**: Прямая интеграция через HTTPS

### DevOps
*   **Контейнеризация**: Docker, Docker Compose
*   **CI/CD**: (В разработке)

---

## API Endpoints

Python FastAPI предоставляет следующие эндпоинты:

### `GET /`
Проверка работоспособности API
```json
{
  "message": "AI Agents Platform API is running"
}
```

### `GET /agents`
Получить список доступных агентов
```json
{
  "agents": ["random", "heuristic", "qlearning", "llm"]
}
```

### `POST /game/play`
Запустить игру между двумя агентами

**Request:**
```json
{
  "agent_x": "heuristic",
  "agent_o": "random",
  "seed": 42
}
```

**Response:**
```json
{
  "winner": "X",
  "steps": 15,
  "slides": [...]  // Полная история игры
}
```

### `POST /train`
Обучить Q-Learning агента (для Крестиков-ноликов)

**Request:**
```json
{
  "agent_type": "qlearning",
  "episodes": 1000,
  "seed": 42,
  "opponent_type": "random"
}
```

**Response:**
```json
{
  "success": true,
  "stats": {
    "wins": 750,
    "losses": 200,
    "draws": 50
  }
}
```

### Mafia API Endpoints

Мафия использует следующие эндпоинты для взаимодействия с C++ движком:

*   `POST /mafia/agent_action`: Запрос действия от агента (убийство, проверка, лечение и т.д.)
*   `POST /mafia/agent_chat`: Запрос текстового сообщения для обсуждения в чате.
*   `GET /agents`: Возвращает список доступных агентов, включая `mafia_random`, `mafia_aggressive`, `citizen_social` и другие.

Подробное описание моделей данных Мафии можно найти в [API документации Python](./python/games/mafia/src/README.md).

---

## Конфигурация

### C++ Bot (`cpp/config.json`)

```json
{
  "bot_token": "YOUR_TELEGRAM_BOT_TOKEN",
  "api_url": "http://python-api:8000"  // Для Docker
  // "api_url": "http://localhost:8000"  // Для локального запуска
}
```

**Как получить токен:**
1. Найдите `@BotFather` в Telegram
2. Отправьте `/newbot`
3. Следуйте инструкциям
4. Скопируйте полученный токен в `config.json`

---

## Docker Compose конфигурация

### Порты
- **8000**: Python API (внешний доступ для отладки)

### Volumes
- `./python/output`: Визуализации игр
- `./python/agents`: Сохраненные модели Q-Learning
- `./cpp/config.json`: Конфигурация бота

### Сети
- `ai-platform`: Внутренняя сеть для взаимодействия контейнеров

### Health Checks
Python API проверяется каждые 30 секунд. Telegram бот ждет готовности API перед запуском.

---

## Команды Docker

```bash
# Запуск в фоне
docker-compose up -d

# Просмотр логов
docker-compose logs -f

# Просмотр логов конкретного сервиса
docker-compose logs -f python-api
docker-compose logs -f telegram-bot

# Остановка
docker-compose down

# Пересборка
docker-compose up --build

# Удаление volumes (очистка данных)
docker-compose down -v
```

---

## Тестирование

### Python
```bash
cd python
python3 -m pytest tests/ -v

# С покрытием
python3 -m pytest tests/ --cov=. --cov-report=term-missing
```

### C++
```bash
cd cpp/build
# Тесты в разработке
```

---

## Разработка и расширение

### Добавление нового агента

1. **Python часть** (`python/agents/my_agent.py`):
```python
from agents.base_agent import BaseAgent

class MyAgent(BaseAgent):
    def select_action(self, board, player_symbol):
        # Ваша логика
        return (row, col)
```

2. **Регистрация в API** (`python/api.py`):
```python
AGENT_CLASSES = {
    "random": RandomAgent,
    "heuristic": HeuristicAgent,
    "qlearning": QLearningAgent,
    "my_agent": MyAgent,  # Добавить здесь
}
```

3. **Обновление C++** - не требуется! Список агентов запрашивается из API.

---

## Статус проекта

- [x] Игровой движок (Tic-Tac-Toe 5x5)
- [x] Игровой движок (Мафия на C++)
- [x] Базовые агенты (Random, Heuristic, RL)
- [x] Специализированные агенты для Мафии (Python)
- [x] Обучаемый агент (Q-Learning)
- [x] REST API (FastAPI)
- [x] Telegram-бот интерфейс (C++)
- [x] Интеграция локальных LLM (Gemma 3) через Ollama
- [x] Логирование "рассуждений" (Chain of Thought) LLM в консоль API
- [x] Система ставок и управления состояниями пользователей
- [x] Визуализация партий (для Крестиков-ноликов)
- [x] Docker контейнеризация
- [x] Микросервисная архитектура (Python API + C++ Client)
- [x] Ролевая модель Мафии (Дон, Шериф, Доктор, Мафия, Мирный)
- [ ] Обучение RL-агентов для Мафии
- [ ] CI/CD pipeline
- [ ] Web интерфейс (в планах)
- [ ] Поддержка других игр (Шахматы/Го)

---

## Troubleshooting

### 🔴 Бот не подключается к API

**Локальная разработка:**
```bash
# Проверьте что API запущен
curl http://localhost:8000/

# Проверьте config.json
cat cpp/config.json  # Должен быть "http://localhost:8000"
```

**Docker:**
```bash
# Проверьте что оба контейнера запущены
docker-compose ps

# Проверьте логи API
docker-compose logs python-api

# Проверьте сеть
docker network inspect courseworkhse_ai-platform
```

### 🔴 API возвращает ошибки

```bash
# Проверьте Python зависимости
cd python
pip install -r requirements.txt

# Запустите тесты
python3 -m pytest tests/ -v
```

### 🔴 C++ бот не собирается

```bash
# Убедитесь что все зависимости установлены
brew list | grep -E "(cmake|jsoncpp|curl)"  # macOS
dpkg -l | grep -E "(cmake|jsoncpp|curl)"    # Ubuntu

# Очистите build директорию
cd cpp
rm -rf build
mkdir build && cd build
cmake ..
make
```

---

## Лицензия

Этот проект создан в рамках курсовой работы НИУ ВШЭ.

---

## Acknowledgments

- Telegram Bot API Documentation
- FastAPI Documentation
- Docker Documentation
- Reinforcement Learning Community
