#!/bin/bash

# Скрипт быстрого развертывания AI Agents Platform
# Использование: ./deploy.sh [start|stop|restart|logs|status]

set -e

COMMAND=${1:-start}

function print_header() {
    echo "=================================="
    echo "$1"
    echo "=================================="
}

function check_config() {
    if [ ! -f "cpp/config.json" ]; then
        echo "⚠️  Конфиг не найден!"
        echo "Копирую пример конфига..."
        cp cpp/config.json.example cpp/config.json
        echo ""
        echo "❗ ВАЖНО: Отредактируйте cpp/config.json и вставьте токен бота!"
        echo "Получите токен от @BotFather в Telegram"
        echo ""
        read -p "Нажмите Enter после настройки токена..."
    fi
    
    # Проверяем что токен был изменен
    if grep -q "YOUR_BOT_TOKEN_HERE" cpp/config.json; then
        echo "❌ Ошибка: Токен не настроен в cpp/config.json"
        echo "Замените YOUR_BOT_TOKEN_HERE на реальный токен от @BotFather"
        exit 1
    fi
}

function start() {
    print_header "🚀 Запуск AI Agents Platform"
    check_config
    
    echo "Сборка и запуск контейнеров..."
    docker-compose up --build -d
    
    echo ""
    echo "✅ Платформа запущена!"
    echo ""
    echo "📊 Проверка статуса:"
    docker-compose ps
    echo ""
    echo "📝 Просмотр логов: ./deploy.sh logs"
    echo "🛑 Остановка: ./deploy.sh stop"
    echo ""
    echo "🔗 API доступен на http://localhost:8000/docs"
    echo "💬 Бот доступен в Telegram"
}

function stop() {
    print_header "🛑 Остановка платформы"
    docker-compose down
    echo "✅ Платформа остановлена"
}

function restart() {
    print_header "🔄 Перезапуск платформы"
    stop
    sleep 2
    start
}

function logs() {
    SERVICE=${2:-}
    if [ -z "$SERVICE" ]; then
        echo "📝 Логи всех сервисов (Ctrl+C для выхода):"
        docker-compose logs -f
    else
        echo "📝 Логи сервиса $SERVICE:"
        docker-compose logs -f "$SERVICE"
    fi
}

function status() {
    print_header "📊 Статус платформы"
    docker-compose ps
    echo ""
    
    # Проверка доступности API
    echo "🔍 Проверка Python API..."
    if curl -s http://localhost:8000/ > /dev/null 2>&1; then
        echo "✅ Python API работает (http://localhost:8000)"
    else
        echo "❌ Python API не доступен"
    fi
    
    echo ""
    echo "📝 Для просмотра логов: ./deploy.sh logs [python-api|telegram-bot]"
}

function help() {
    cat << EOF
🤖 AI Agents Platform - Deployment Script

Использование:
  ./deploy.sh [COMMAND]

Команды:
  start     - Запустить платформу (по умолчанию)
  stop      - Остановить платформу
  restart   - Перезапустить платформу
  logs      - Показать логи (Ctrl+C для выхода)
              ./deploy.sh logs python-api - логи только Python API
              ./deploy.sh logs telegram-bot - логи только бота
  status    - Показать статус сервисов
  help      - Показать эту справку

Примеры:
  ./deploy.sh                    # Запуск
  ./deploy.sh start             # Запуск
  ./deploy.sh logs              # Все логи
  ./deploy.sh logs python-api   # Логи API
  ./deploy.sh status            # Статус
  ./deploy.sh stop              # Остановка
EOF
}

case "$COMMAND" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        restart
        ;;
    logs)
        logs "$@"
        ;;
    status)
        status
        ;;
    help|--help|-h)
        help
        ;;
    *)
        echo "❌ Неизвестная команда: $COMMAND"
        echo "Используйте './deploy.sh help' для справки"
        exit 1
        ;;
esac
