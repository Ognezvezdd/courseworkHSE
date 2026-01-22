#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <string>
#include <vector>
#include "json/json.h"

class Keyboard {
public:
    static std::string createMainMenu();
    static std::string createAgentsMenu();
    static std::string createGamesMenu();
    static std::string createBetsMenu();
    static std::string createPlayMenu();
    static std::string createBackMenu();
    static std::string removeKeyboard();

private:
    static std::string createReplyKeyboard(const std::vector<std::vector<std::string>>& buttons, 
                                         bool resize = true);
    static std::string createInlineKeyboard(const std::vector<std::vector<std::pair<std::string, std::string>>>& buttons);
};

#endif