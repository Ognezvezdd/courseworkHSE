#include "keyboard.hpp"
#include <sstream>

using namespace std;

string Keyboard::createMainMenu() {
    vector<vector<string>> buttons = {
        {"Выбрать игру"},
        {"Выбрать агента"},
        {"Сделать ставку"},
        {"Начать игру"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createAgentsMenu() {
    vector<vector<string>> buttons = {
        {"Random (случайный)"},
        {"Heuristic (умный)"},
        {"QLearning (обучаемый)"},
        {"Назад в меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createGamesMenu() {
    vector<vector<string>> buttons = {
        {"Крестики-нолики 5x5"},
        {"Назад в меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createBetsMenu() {
    vector<vector<string>> buttons = {
        {"10", "50", "100"},
        {"500", "1000"},
        {"Назад в меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createPlayMenu() {
    vector<vector<string>> buttons = {
        {"Случайный противник"},
        {"Против Heuristic"},
        {"Против QLearning"},
        {"Назад в меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createBackMenu() {
    vector<vector<string>> buttons = {
        {"Назад в меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createReplyKeyboard(const vector<vector<string>>& buttons, bool resize) {
    Json::Value keyboard(Json::arrayValue);
    
    for (const auto& row : buttons) {
        Json::Value keyboardRow(Json::arrayValue);
        for (const auto& text : row) {
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
