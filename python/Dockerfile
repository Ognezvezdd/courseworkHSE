# Платформа симуляции AI-агентов
# Python 3.11 — минимальный образ
FROM python:3.11-slim

# Метаданные
LABEL maintainer="HSE Student"
LABEL description="AI Agents Platform — 5x5 Tic-Tac-Toe Simulation"

# Рабочая директория
WORKDIR /app

# Копирование зависимостей
COPY requirements.txt .

# Установка зависимостей
RUN pip install --no-cache-dir -r requirements.txt

# Копирование исходного кода
COPY . .

# Переменные окружения
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1

# Команда по умолчанию — демонстрация
CMD ["python", "run.py", "--agent-x", "heuristic", "--agent-o", "random", "--seed", "42"]
