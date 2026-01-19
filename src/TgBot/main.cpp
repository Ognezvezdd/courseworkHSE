#include "bot.hpp"
#include <iostream>

int main() {
    std::cout << "=== Telegram Bot HSE Coursework ===" << std::endl;
    std::cout << "Бот с меню выбора агента, игры и ставки" << std::endl;
    
    // токен от BotFather, потом получим его, пока с остальным определимся
    std::string bot_token = "ВАШ_TELEGRAM_BOT_TOKEN_ЗДЕСЬ";
    
    if (bot_token == "ВАШ_TELEGRAM_BOT_TOKEN_ЗДЕСЬ") {
    
    try {
        TelegramBot bot(bot_token);
        bot.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}