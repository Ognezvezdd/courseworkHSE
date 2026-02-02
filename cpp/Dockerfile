# C++ Telegram Bot
FROM ubuntu:22.04

# Метаданные
LABEL maintainer="HSE Student"
LABEL description="Telegram Bot for AI Agents Platform"

# Установка переменных окружения для неинтерактивной установки
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow

# Установка необходимых пакетов и библиотек
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    libssl-dev \
    libboost-all-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Рабочая директория
WORKDIR /app

# Копирование исходного кода
COPY . .

# Создание директории сборки и компиляция
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

# Создание конфигурационного файла по умолчанию (будет перезаписан через volume)
RUN echo '{"bot_token":"","api_url":"http://python-api:8000"}' > /app/config.json

# Команда запуска
CMD ["./build/tg_bot"]
