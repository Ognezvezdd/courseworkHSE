#include "keyboard.hpp"
#include <sstream>

using namespace std;

string Keyboard::createMainMenu() {
  vector<vector<string>> buttons = {{"🎮 Крестики-нолики"},
                                    {"🎭 Мафия"},
                                    {"🛡️ Бункер"},
                                    {"📊 Статистика"},
                                    {"⚙️ Настройки"}};
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
                                    {"👥 Выбор агентов"},
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

string Keyboard::createMafiaAgentsMenu(const vector<string> &available,
                                       const vector<string> &selected) {
  vector<vector<string>> buttons;

  for (const auto &agent : available) {
    bool is_selected = false;
    for (const auto &s : selected) {
      if (s == agent) {
        is_selected = true;
        break;
      }
    }

    string label = (is_selected ? "✅ " : "") + agent;
    buttons.push_back({label});
  }

  buttons.push_back({"🔄 Очистить выбор"});
  buttons.push_back({"◀️ Назад"});

  return createReplyKeyboard(buttons);
}

// === Бункер ===

string Keyboard::createBunkerMenu() {
  vector<vector<string>> buttons = {{"🏚️ Вместимость бункера"},
                                    {"📋 Правила бункера"},
                                    {"▶️ Запустить бункер"},
                                    {"◀️ Назад в главное меню"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createBunkerCapacityMenu() {
  vector<vector<string>> buttons = {{"Вместимость 3"},
                                    {"Вместимость 4"},
                                    {"Вместимость 5"},
                                    {"Вместимость 6"},
                                    {"◀️ Назад"}};
  return createReplyKeyboard(buttons);
}

string Keyboard::createSettingsMenu() {
  vector<vector<string>> buttons = {{"🔑 Установить OpenAI Key"},
                                    {"🗑️ Сбросить ключ"},
                                    {"◀️ Назад в главное меню"}};
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
  writer["emitUTF8"] = true;
  return Json::writeString(writer, replyMarkup);
}

string Keyboard::removeKeyboard() {
  Json::Value replyMarkup;
  replyMarkup["remove_keyboard"] = true;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  writer["emitUTF8"] = true;
  return Json::writeString(writer, replyMarkup);
}