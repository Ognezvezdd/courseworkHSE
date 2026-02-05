#include "keyboard.hpp"
#include <sstream>

using namespace std;

string Keyboard::createMainMenu() {
    vector<vector<string>> buttons = {
        {"🎮 Крестики-нолики"},
        {"🎭 Мафия"},
        {"📊 Статистика"},
        {"⚙️ Настройки"}
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
        {"🎭 Перейти к мафии"},
        {"Назад в меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createBetsMenu() {
    vector<vector<string>> buttons = {
        {"10", "50", "100"},
        {"500", "1000", "5000"},
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

string Keyboard::createMafiaMenu() {
    vector<vector<string>> buttons = {
        {"🎭 Начать игру в мафию"},
        {"👥 Выбрать количество игроков"},
        {"⚙️ Настройки мафии"},
        {"📋 Правила мафии"},
        {"Назад в главное меню"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createMafiaAgentsMenu() {
    vector<vector<string>> buttons = {
        {"6 игроков", "7 игроков", "8 игроков"},
        {"9 игроков", "10 игроков", "12 игроков"},
        {"Случайное количество"},
        {"Назад в меню мафии"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createMafiaSettingsMenu() {
    vector<vector<string>> buttons = {
        {"💬 Включить чат", "🔇 Без чата"},
        {"⚡ Быстрая игра", "🕐 Стандартная"},
        {"🎲 Случайные роли", "⚖️ Сбалансированные"},
        {"Назад в меню мафии"}
    };
    return createReplyKeyboard(buttons);
}

string Keyboard::createMafiaPlayMenu() {
    vector<vector<string>> buttons = {
        {"▶️ Начать игру сейчас"},
        {"👥 Выбрать своих агентов"},
       {"💰 Сделать ставку"},
        {"Назад в меню мафии"}
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