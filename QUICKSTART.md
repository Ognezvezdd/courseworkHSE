# 🚀 Быстрый старт: AI Agents Platform

## За 3 минуты до запуска!

### Шаг 1: Получите Telegram Bot Token

1. Откройте Telegram и найдите `@BotFather`
2. Отправьте команду `/newbot`
3. Следуйте инструкциям (придумайте имя и username для бота)
4. Скопируйте полученный токен (выглядит как `123456789:ABCdefGHIjklMNOpqrsTUVwxyz`)

### Шаг 2: Настройте конфигурацию

```bash
# Скопируйте пример конфига
cp cpp/config.json.example cpp/config.json

# Отредактируйте файл (можно в любом редакторе)
nano cpp/config.json
# или
code cpp/config.json
# или
vim cpp/config.json
```

Замените `YOUR_BOT_TOKEN_HERE` на ваш токен от BotFather:

```json
{
    "bot_token": "123456789:ABCdefGHIjklMNOpqrsTUVwxyz",
    "api_url": "http://python-api:8000"
}
```

### Шаг 3: Запустите платформу

```bash
# Простой способ - используйте скрипт
./deploy.sh

# Или вручную
docker-compose up --build
```

### Шаг 4: Проверьте работу

1. **Откройте Telegram** и найдите вашего бота по username
2. **Отправьте `/start`** - бот должен ответить
3. **Откройте браузер** на http://localhost:8000/docs - API документация

### 🎉 Готово!

Ваша платформа запущена! Теперь можете:
- 🎮 Играть через Telegram бота
- 📊 Тестировать API на http://localhost:8000/docs
- 🤖 Смотреть соревнования AI агентов

---

## Управление платформой

### Просмотр логов

```bash
# Все логи
./deploy.sh logs

# Только Python API
./deploy.sh logs python-api

# Только Telegram бот
./deploy.sh logs telegram-bot
```

### Остановка

```bash
./deploy.sh stop

# Или
docker-compose down
```

### Перезапуск

```bash
./deploy.sh restart

# Или
docker-compose restart
```

### Статус

```bash
./deploy.sh status

# Или
docker-compose ps
```

---

## Возможные проблемы

### ❌ "Bot token not set"

**Решение:** Проверьте что вы заменили `YOUR_BOT_TOKEN_HERE` в `cpp/config.json`

### ❌ "API не доступен"

**Решение:** 
```bash
# Проверьте логи Python API
docker-compose logs python-api

# Убедитесь что оба контейнера запущены
docker-compose ps
```

### ❌ "Error response from daemon"

**Решение:** Убедитесь что Docker запущен:
```bash
# macOS
open /Applications/Docker.app

# Linux
sudo systemctl start docker
```

---

## Локальная разработка (без Docker)

### Python API

```bash
cd python
pip install -r requirements.txt
uvicorn api:app --reload --host 0.0.0.0 --port 8000
```

### C++ Bot

```bash
cd cpp

# macOS
brew install cmake jsoncpp libcurl

# Ubuntu
sudo apt-get install cmake libcurl4-openssl-dev libjsoncpp-dev

# Сборка
mkdir build && cd build
cmake ..
make

# Отредактируйте config.json (api_url: "http://localhost:8000")
nano ../config.json

# Запуск
./tg_bot
```

---

## Следующие шаги

1. 📖 Прочитайте [полный README](README.md)
2. 🐍 Изучите [Python документацию](python/README.md)
3. ⚙️ Изучите [C++ документацию](cpp/README.md)
4. 🎯 Попробуйте создать своего агента!

---

**Вопросы?** Открывайте Issue на GitHub или обращайтесь к авторам!
