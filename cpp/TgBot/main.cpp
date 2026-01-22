#include "bot.hpp"
#include "config.hpp"
#include "game_manager.hpp"
#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "=== Telegram Bot HSE Coursework ===" << std::endl;
    std::cout << "Бот с AI-агентами для игры в крестики-нолики" << std::endl;
    
    // Загрузка конфигурации
    Config config = load_config("config.json");
    
    if (config.bot_token == "YOUR_BOT_TOKEN_HERE") {
        std::cerr << "ERROR: Please set your bot token in config.json" << std::endl;
        std::cerr << "Get token from @BotFather on Telegram" << std::endl;
        return 1;
    }
    
    // Проверка Python и скрипта
    std::cout << "Checking Python script..." << std::endl;
    GameManager game_manager(config.python_path, config.game_script_path);
    
    // Тестовый запуск Python
    auto agents = GameManager::getAvailableAgents();
    std::cout << "Available agents: ";
    for (const auto& agent : agents) {
        std::cout << agent << " ";
    }
    std::cout << std::endl;
    
    try {
        TelegramBot bot(config.bot_token, game_manager);
        std::cout << "Bot starting..." << std::endl;
        bot.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}