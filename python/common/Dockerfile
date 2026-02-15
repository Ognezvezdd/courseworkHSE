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

# Expose API port
EXPOSE 8000

# Запуск FastAPI сервера через uvicorn
CMD ["uvicorn", "api:app", "--host", "0.0.0.0", "--port", "8000"]
