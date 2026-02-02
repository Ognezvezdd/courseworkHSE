# 🎯 Интеграция Python API и C++ Telegram Bot

## Что было сделано

### 📋 Краткое описание

Реализована микросервисная архитектура для платформы AI Agents Platform:
- **Python FastAPI** - игровой движок и REST API
- **C++ Telegram Bot** - клиентский интерфейс
- **Docker Compose** - оркестрация сервисов

### 🏗️ Архитектура

#### Было (до рефакторинга):
```
C++ Bot → executes → Python Script (run_game.py)
   ↓
Direct process execution (popen)
```

**Проблемы старого подхода:**
- ❌ Тесная связанность компонентов
- ❌ Сложность развертывания
- ❌ Невозможность масштабирования
- ❌ Сложность тестирования
- ❌ Зависимость от локальных путей

#### Стало (новая архитектура):
```
┌──────────────┐    HTTP/REST    ┌──────────────┐
│  C++ Bot     │ ◀──────────────▶ │  Python API  │
│ (Container)  │    JSON         │ (Container)  │
└──────────────┘                  └──────────────┘
      │                                  │
      │                                  │
      ▼                                  ▼
 Telegram Users                   AI Agents Engine
```

**Преимущества новой архитектуры:**
- ✅ Микросервисная архитектура
- ✅ Независимое развертывание сервисов
- ✅ Легкое масштабирование (можно запустить несколько инстансов)
- ✅ Простое тестирование (каждый сервис отдельно)
- ✅ Единственная точка входа через API
- ✅ Один Docker Compose для запуска всего

---

## 📦 Новые файлы

### Python
1. **`python/Dockerfile`** (обновлен)
   - Настроен на запуск uvicorn вместо demo скрипта
   - CMD: `uvicorn api:app --host 0.0.0.0 --port 8000`

2. **`python/.dockerignore`**
   - Оптимизация сборки Docker образа
   - Исключение ненужных файлов (cache, output, etc.)

### C++
1. **`cpp/Dockerfile`** (новый)
   - Ubuntu 22.04 base
   - Установка всех зависимостей (cmake, curl, jsoncpp)
   - Автоматическая компиляция проекта

2. **`cpp/src/http_client.hpp`** (новый)
   - HTTP клиент на основе libcurl
   - Методы: GET, POST с JSON телом

3. **`cpp/src/http_client.cpp`** (новый)
   - Реализация HTTP клиента
   - Поддержка JSON headers
   - Error handling

4. **`cpp/src/game_manager.hpp`** (полностью переписан)
   - Убраны python_path и script_path
   - Добавлен api_url
   - Новые методы: checkApiHealth(), getAvailableAgents()

5. **`cpp/src/game_manager.cpp`** (полностью переписан)
   - Все вызовы через HTTP API
   - JSON парсинг ответов
   - Взаимодействие через HttpClient

6. **`cpp/src/config.hpp`** (обновлен)
   - Структура Config теперь содержит только bot_token и api_url

7. **`cpp/src/config.cpp`** (обновлен)
   - Загрузка api_url вместо путей к Python

8. **`cpp/TgBot/main.cpp`** (обновлен)
   - Инициализация GameManager с api_url
   - Проверка здоровья API при старте
   - Получение списка агентов от API

9. **`cpp/CMakeLists.txt`** (обновлен)
   - Добавлен http_client.cpp в SOURCES

10. **`cpp/config.json`** (обновлен)
    - Новый формат: `{"bot_token": "...", "api_url": "..."}`

11. **`cpp/config.json.example`** (новый)
    - Шаблон конфигурации для новых пользователей

12. **`cpp/.dockerignore`** (новый)
    - Оптимизация сборки

### Root directory
1. **`docker-compose.yml`** (новый)
   - Определение двух сервисов: python-api и telegram-bot
   - Настройка сети ai-platform
   - Health checks для python-api
   - Правильная последовательность запуска (depends_on)
   - Монтирование volumes для persistence

2. **`README.md`** (полностью переписан)
   - Подробное описание новой архитектуры
   - API endpoints документация
   - Docker Compose инструкции
   - Troubleshooting секция
   - Примеры использования

3. **`QUICKSTART.md`** (новый)
   - Пошаговая инструкция для начинающих
   - 3-минутный старт
   - Решение типичных проблем

4. **`deploy.sh`** (новый)
   - Bash скрипт для управления платформой
   - Команды: start, stop, restart, logs, status
   - Автоматическая проверка конфигурации

5. **`Makefile`** (новый)
   - Make команды для удобства
   - Цели: build, up, down, logs, test, clean
   - Цветной вывод
   - Help команда

6. **`.gitignore`** (обновлен)
   - Расширенный список исключений
   - Защита config.json с секретами
   - Исключение build артефактов

---

## 🔧 Изменения в существующих файлах

### Python
- **`api.py`** - уже был готов, никаких изменений не требуется
- **`Dockerfile`** - изменена CMD команда на uvicorn

### C++
- **Все файлы в `src/`** - полностью переписаны для HTTP API
- **`TgBot/main.cpp`** - обновлен для работы с новой архитектурой
- **`CMakeLists.txt`** - добавлен http_client.cpp

---

## 🚀 Как использовать

### Вариант 1: Docker Compose (рекомендуется)

```bash
# 1. Настройте токен
cp cpp/config.json.example cpp/config.json
# Отредактируйте cpp/config.json

# 2. Запустите
docker-compose up --build

# Или используйте удобные скрипты:
./deploy.sh start
# или
make up
```

### Вариант 2: Локальная разработка

**Terminal 1 - Python API:**
```bash
cd python
pip install -r requirements.txt
uvicorn api:app --reload --host 0.0.0.0 --port 8000
```

**Terminal 2 - C++ Bot:**
```bash
cd cpp
mkdir build && cd build
cmake ..
make
./tg_bot
```

---

## 📊 API Endpoints

### `GET /`
Проверка здоровья

### `GET /agents`
Список доступных агентов

### `POST /game/play`
Запуск игры
```json
{
  "agent_x": "heuristic",
  "agent_o": "random",
  "seed": 42
}
```

### `POST /train`
Обучение Q-Learning агента
```json
{
  "agent_type": "qlearning",
  "episodes": 1000,
  "seed": 42,
  "opponent_type": "random"
}
```

---

## 🎯 Ключевые улучшения

### Разделение ответственности
- Python - только игровая логика и AI
- C++ - только Telegram интерфейс
- Четкий контракт через REST API

### Масштабируемость
- Можно запустить несколько инстансов бота
- Можно запустить несколько инстансов API
- Load balancing через nginx (в будущем)

### Тестируемость
- Python API можно тестировать через curl/Postman
- C++ бот можно тестировать с mock API
- Unit тесты для каждого компонента независимо

### Развертывание
- Одна команда для запуска всего: `docker-compose up`
- Автоматическая сборка и настройка
- Health checks и зависимости между сервисами

### DevOps
- Готово для CI/CD
- Легко добавить monitoring (Prometheus/Grafana)
- Легко добавить logging (ELK stack)

---

## 🔍 Что проверить

### 1. Python API работает
```bash
curl http://localhost:8000/
curl http://localhost:8000/agents
```

### 2. C++ бот подключается к API
```bash
docker-compose logs telegram-bot | grep "API доступен"
```

### 3. Telegram бот отвечает
Отправьте `/start` вашему боту в Telegram

### 4. Игра запускается
Используйте меню бота для выбора агентов и запуска игры

---

## 📈 Следующие шаги (опционально)

1. **CI/CD**: GitHub Actions для автоматического тестирования и деплоя
2. **Monitoring**: Prometheus + Grafana для мониторинга
3. **Logging**: ELK stack для централизованных логов
4. **Load Balancing**: nginx для распределения нагрузки
5. **Database**: PostgreSQL для хранения истории игр
6. **Web UI**: React фронтенд для просмотра игр в браузере

---

## ✅ Чеклист для финальной проверки

- [x] Python API запускается в Docker
- [x] C++ бот запускается в Docker
- [x] Бот подключается к API
- [x] API endpoints работают
- [x] Telegram бот отвечает на команды
- [x] Игры запускаются через бота
- [x] Docker Compose работает
- [x] Документация обновлена
- [x] README содержит инструкции
- [x] Скрипты деплоя работают

---

## 💡 Tips

### Просмотр логов
```bash
# Все логи
docker-compose logs -f

# Только API
docker-compose logs -f python-api

# Только бот
docker-compose logs -f telegram-bot
```

### Перезапуск после изменений
```bash
# Пересборка и запуск
docker-compose up --build

# Или
make restart
```

### Тестирование API
```bash
# Swagger UI
open http://localhost:8000/docs

# curl
curl -X POST http://localhost:8000/game/play \
  -H "Content-Type: application/json" \
  -d '{"agent_x":"heuristic","agent_o":"random","seed":42}'
```

---

**✨ Готово! Платформа полностью интегрирована и готова к использованию!**
