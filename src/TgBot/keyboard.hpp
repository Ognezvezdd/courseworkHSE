#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <string>
#include <vector>
#include "json/json.h"

class Keyboard {
public:
    // Основное меню
    static std::string createMainMenu();
    
    // Меню выбора агента
    static std::string createAgentsMenu();
    
    // Меню выбора игры
    static std::string createGamesMenu();
    
    // Меню выбора ставки
    static std::string createBetsMenu();
    
    // Удалить клавиатуру
    static std::string removeKeyboard();

private:
    static std::string createReplyKeyboard(const std::vector<std::vector<std::string>>& buttons, bool resize = true);
};

#endif