#include "keyboard.hpp"
#include <sstream>

using namespace std;

string Keyboard::createMainMenu() {
  vector<vector<string>> buttons = {
      {"🎮 Крестики-нолики"}, {"🎭 Мафия"}, {"🛡️ Бункер"}, {"📊 Статистика"}, {"⚙️ Настройки"}};
  return createReplyKeyboard(buttons);
}

// === Крестики-нолики ===

string Keyboard::createTicTacToeMenu() {
  vector<vector<string>> buttons = {{"🤖 Выбрать агента"},
                                    {"🎯 Выбрать противника"},
                                    {"▶️ Запустить игру"},
                                    {"◀️ Назад в главное меню"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createTTTAgentMenu() {
  vector<vector<string>> buttons = {
      {"Random"}, {"Heuristic"}, {"QLearning"}, {"LLM"}, {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createTTTOpponentMenu() {
  vector<vector<string>> buttons = {{"Противник: Random"},
                                    {"Противник: Heuristic"},
                                    {"Противник: QLearning"},
                                    {"Противник: LLM"},
                                    {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

// === Мафия ===

string Keyboard::createMafiaMenu() {
  vector<vector<string>> buttons = {{"👥 Количество игроков"},
                                    {"📋 Правила"},
                                    {"▶️ Запустить мафию"},
                                    {"◀️ Назад в главное меню"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createMafiaPlayersMenu() {
  vector<vector<string>> buttons = {
      {"6 игроков"},  {"7 игроков"},  {"8 игроков"}, {"9 игроков"},
      {"10 игроков"}, {"12 игроков"}, {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createMafiaAgentsMenu() {
  vector<vector<string>> buttons = {{"mafia_random"},
                                    {"mafia_aggressive"},
                                    {"citizen_social"},
                                    {"citizen_cautious"},
                                    {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

// === Бункер ===

string Keyboard::createBunkerMenu() {
  vector<vector<string>> buttons = {{"🏚️ Вместимость бункера"},
                                    {"💰 Ставка бункер"},
                                    {"📋 Правила бункера"},
                                    {"▶️ Запустить бункер"},
                                    {"◀️ Назад в главное меню"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createBunkerCapacityMenu() {
  vector<vector<string>> buttons = {{"Вместимость 3"}, {"Вместимость 4"},
                                    {"Вместимость 5"}, {"Вместимость 6"},
                                    {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createBunkerBetsMenu() {
  vector<vector<string>> buttons = {
      {"Ставка бункер 10"},  {"Ставка бункер 50"},   {"Ставка бункер 100"},
      {"Ставка бункер 500"}, {"Ставка бункер 1000"}, {"Ставка бункер 5000"},
      {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

// === Общие ===

string Keyboard::createReplyKeyboard(const vector<vector<string>> &buttons,
                                     bool resize) {
  Json::Value keyboard(Json::arrayValue);

  for (const auto &row : buttons) {
    Json::Value keyboardRow(Json::arrayValue);
    for (const auto &text : row) {
      Json::Value button;
      button["text"] = text;
      keyboardRow.append(button);
    }
    keyboard.append(keyboardRow);
  }

  Json::Value replyMarkup;
  replyMarkup["keyboard"] = keyboard;
  replyMarkup["resize_keyboard"] = resize;
  replyMarkup["one_time_keyboard"] = false;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  return Json::writeString(writer, replyMarkup);
}

string Keyboard::removeKeyboard() {
  Json::Value replyMarkup;
  replyMarkup["remove_keyboard"] = true;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  return Json::writeString(writer, replyMarkup);
}