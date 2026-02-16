#include "keyboard.hpp"
#include <sstream>

using namespace std;

string Keyboard::createMainMenu() {
  vector<vector<string>> buttons = {
      {"🎮 Крестики-нолики"}, {"🎭 Мафия"}, {"📊 Статистика"}, {"⚙️ Настройки"}};
  return createReplyKeyboard(buttons);
}

// === Крестики-нолики ===

string Keyboard::createTicTacToeMenu() {
  vector<vector<string>> buttons = {{"🤖 Выбрать агента"},
                                    {"🎯 Выбрать противника"},
                                    {"💰 Ставка"},
                                    {"▶️ Запустить игру"},
                                    {"◀️ Назад в главное меню"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createTTTAgentMenu() {
  vector<vector<string>> buttons = {
      {"Random"}, {"Heuristic"}, {"QLearning"}, {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createTTTOpponentMenu() {
  vector<vector<string>> buttons = {{"Противник: Random"},
                                    {"Противник: Heuristic"},
                                    {"Противник: QLearning"},
                                    {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createTTTBetsMenu() {
  vector<vector<string>> buttons = {
      {"Ставка 10"},   {"Ставка 50"},   {"Ставка 100"}, {"Ставка 500"},
      {"Ставка 1000"}, {"Ставка 5000"}, {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

// === Мафия ===

string Keyboard::createMafiaMenu() {
  vector<vector<string>> buttons = {{"👥 Количество игроков"},
                                    {"💰 Ставка мафия"},
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

string Keyboard::createMafiaBetsMenu() {
  vector<vector<string>> buttons = {
      {"Ставка мафия 10"},  {"Ставка мафия 50"},   {"Ставка мафия 100"},
      {"Ставка мафия 500"}, {"Ставка мафия 1000"}, {"Ставка мафия 5000"},
      {"◀️ Назад"}};
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