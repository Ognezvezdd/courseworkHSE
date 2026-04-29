.PHONY: help build up down restart logs logs-api logs-bot status clean test

# Цвета для вывода
CYAN := \033[0;36m
GREEN := \033[0;32m
RESET := \033[0m

help: ## Показать эту справку
	@echo "$(CYAN)AI Agents Platform - Make команды$(RESET)"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  $(GREEN)%-15s$(RESET) %s\n", $$1, $$2}'
	@echo ""

check-config: ## Проверить конфигурацию
	@if [ ! -f cpp/config.json ]; then \
		echo "⚠️  Config not found! Copying example..."; \
		cp cpp/config.json.example cpp/config.json; \
		echo "❗ Edit cpp/config.json and set your bot token!"; \
		exit 1; \
	fi
	@if grep -q "YOUR_BOT_TOKEN_HERE" cpp/config.json; then \
		echo "❌ Bot token not configured in cpp/config.json"; \
		exit 1; \
	fi
	@echo "✅ Config OK"

build: check-config ## Собрать Docker образы
	docker-compose build

up: check-config ## Запустить всю платформу
	docker-compose up -d
	@echo ""
	@echo "$(GREEN)✅ Platform started!$(RESET)"
	@echo "📊 API docs: http://localhost:8000/docs"
	@echo "💬 Bot is running in Telegram"
	@echo ""
	@make status

down: ## Остановить платформу
	docker-compose down
	@echo "$(GREEN)✅ Platform stopped$(RESET)"

restart: ## Перезапустить платформу
	@make down
	@sleep 2
	@make up

logs: ## Показать логи всех сервисов
	docker-compose logs -f

logs-api: ## Показать логи Python API
	docker-compose logs -f python-api

logs-bot: ## Показать логи Telegram бота
	docker-compose logs -f telegram-bot

status: ## Показать статус сервисов
	@echo "$(CYAN)Service Status:$(RESET)"
	@docker-compose ps
	@echo ""
	@echo "$(CYAN)API Health:$(RESET)"
	@curl -s http://localhost:8000/ | grep -q "running" && echo "✅ API is running" || echo "❌ API is down"

clean: ## Очистить все (контейнеры, образы, volumes)
	docker-compose down -v
	docker-compose rm -f
	@echo "$(GREEN)✅ Cleaned$(RESET)"

test-python: ## Запустить Python тесты игр
	cd python && python3 -m pytest games/tictactoe/src/tests games/mafia/src/tests games/bunker/src/tests -v

test-api: ## Тестировать API endpoints
	@echo "Testing API endpoints..."
	@curl -s http://localhost:8000/ | jq . || echo "Install jq for formatted output"
	@curl -s http://localhost:8000/agents | jq . || echo ""

dev-api: ## Запустить Python API локально (для разработки)
	cd python && python3 -m uvicorn main_service:app --reload --host 0.0.0.0 --port 8000

dev-bot: ## Собрать C++ бота локально
	@if [ ! -d cpp/build ]; then mkdir cpp/build; fi
	cd cpp/build && cmake .. && make
	@echo "$(GREEN)✅ Bot built successfully!$(RESET)"
	@echo "Run with: cd cpp/build && ./tg_bot"

install-deps-mac: ## Установить зависимости на macOS
	brew install cmake jsoncpp libcurl docker docker-compose

install-deps-ubuntu: ## Установить зависимости на Ubuntu
	sudo apt-get update
	sudo apt-get install -y cmake libcurl4-openssl-dev libjsoncpp-dev build-essential docker.io docker-compose
	sudo systemctl start docker
	sudo usermod -aG docker $$USER

# Default target
.DEFAULT_GOAL := help
