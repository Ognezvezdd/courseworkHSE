# Платформа симуляции AI-агентов
# Python 3.11 — минимальный образ
FROM python:3.11-slim

# Метаданные
LABEL maintainer="HSE Student"
LABEL description="AI Agents Platform — 5x5 Tic-Tac-Toe Simulation"

# Рабочая директория
WORKDIR /app

# Копирование зависимостей
# Копируем зависимости
COPY common/requirements.txt .

# Установка зависимостей
RUN pip install --no-cache-dir -r requirements.txt

# Копируем весь код (включая common, games, visualization)
COPY . .

# Переменные окружения
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONPATH=/app

# Expose API port
EXPOSE 8000

# Запуск FastAPI сервера через uvicorn
# Запускаем main_service.py как модуль
CMD ["uvicorn", "main_service:app", "--host", "0.0.0.0", "--port", "8000"]
