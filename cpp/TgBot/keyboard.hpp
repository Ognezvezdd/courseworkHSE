#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "json/json.h"
#include <string>
#include <vector>

class Keyboard {
public:
  // Главное меню
  static std::string createMainMenu();

  // Крестики-нолики
  static std::string createTicTacToeMenu();
  static std::string createTTTAgentMenu();
  static std::string createTTTOpponentMenu();
  static std::string createTTTBetsMenu();

  // Мафия
  static std::string createMafiaMenu();
  static std::string createMafiaPlayersMenu();
  static std::string createMafiaBetsMenu();
  static std::string createMafiaAgentsMenu();

  // Общие
  static std::string removeKeyboard();

private:
  static std::string
  createReplyKeyboard(const std::vector<std::vector<std::string>> &buttons,
                      bool resize = true);
};

#endif // KEYBOARD_HPP