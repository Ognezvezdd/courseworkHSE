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

  // Мафия
  static std::string createMafiaMenu();
  static std::string createMafiaPlayersMenu();
  static std::string createMafiaAgentsMenu();
  
  // Бункер
  static std::string createBunkerMenu();
  static std::string createBunkerCapacityMenu();
  static std::string createBunkerBetsMenu();

  // Общие
  static std::string removeKeyboard();

private:
  static std::string
  createReplyKeyboard(const std::vector<std::vector<std::string>> &buttons,
                      bool resize = true);
};

#endif // KEYBOARD_HPP