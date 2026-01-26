#include "bot.hpp"
#include "config.hpp"
#include "game_manager.hpp"
#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "=== Telegram Bot HSE Coursework ===" << std::endl;
    std::cout << "Бот с AI-агентами для игры в крестики-нолики" << std::endl;

    Config config = load_config("config.json");
    
    if (config.bot_token == "YOUR_BOT_TOKEN_HERE") {
        std::cerr << "ERROR: Please set your bot token in config.json" << std::endl;
        std::cerr << "Get token from @BotFather on Telegram" << std::endl; //need
        std::cerr << "Format: {\"bot_token\": \"YOUR_TOKEN_HERE\"}" << std::endl;
        return 1;
    }
    
    if (config.game_script_path.empty()) {
        config.game_script_path = "../python/run_game.py";
    }

    std::cout << "Python path: " << config.python_path << std::endl;
    std::cout << "Script path: " << config.game_script_path << std::endl;
    
    GameManager game_manager(config.python_path, config.game_script_path);

    std::cout << "\nПроверка доступности агентов..." << std::endl;
    try {
        auto agents = GameManager::getAvailableAgents();
        std::cout << "Доступные агенты: ";
        for (const auto& agent : agents) {
            std::cout << agent << " ";
        }
        std::cout << std::endl;

        std::cout << "\nТестовый запуск игры..." << std::endl;
        GameResult test_result = game_manager.runGame("random", "random", 42);
        std::cout << "Результат теста: " << test_result.winner 
                  << " (ходов: " << test_result.steps << ")" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Предупреждение: " << e.what() << std::endl;
        std::cerr << "Продолжаем запуск бота..." << std::endl;
    }

    try {
        TelegramBot bot(config.bot_token, game_manager);
        std::cout << "\n✅ Бот запускается..." << std::endl;
        std::cout << "Для остановки нажмите Ctrl+C" << std::endl;
        bot.run();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}