#include "bot.hpp"
#include "keyboard.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <iostream>
#include <sstream>

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

TelegramBot::TelegramBot(const string& token) : token_(token) {
    base_url_ = "https://api.telegram.org/bot" + token + "/";
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

TelegramBot::~TelegramBot() {
    curl_global_cleanup();
}

string TelegramBot::makeRequest(const string& method, const string& params) {
    CURL* curl = curl_easy_init();
    string response;
    
    if (curl) {
        string url = base_url_ + method;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "CURL error: " << curl_easy_strerror(res) << endl;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

void TelegramBot::sendMessage(int64_t chat_id, const string& text, const string& reply_markup) {
    Json::Value params;
    params["chat_id"] = chat_id;
    params["text"] = text;
    
    if (!reply_markup.empty()) {
        params["reply_markup"] = reply_markup;
    }
    
    Json::StreamWriterBuilder writer;
    string json_params = Json::writeString(writer, params);
    
    makeRequest("sendMessage", json_params);
}

void TelegramBot::showMainMenu(int64_t chat_id, const string& username) {
    string welcome = "🎮 *Добро пожаловать, " + username + "!*\n\n"
                    "Выберите действие:\n\n"
                    "*Выбрать игру* - выбор игры для ставки\n"
                    "*Выбрать агента* - выбор агента для игры\n"
                    "*Сделать ставку* - установить размер ставки\n"
                    //"*Статистика* - ваша статистика\n"
                    //"*Баланс* - текущий баланс\n"
                    //"*Помощь* - как пользоваться ботом";
    
    sendMessage(chat_id, welcome, Keyboard::createMainMenu());
    user_states_[chat_id] = "main_menu";
}

void TelegramBot::showAgentsMenu(int64_t chat_id) {
    string agents_text = "*ВЫБОР АГЕНТА*\n\n"
                        "Выберите агента для игры:\n\n"
                        //пока не очев совсем тут, хз, что за агенты и что писать о них

    sendMessage(chat_id, agents_text, Keyboard::createAgentsMenu());
    user_states_[chat_id] = "choose_agent";
}

void TelegramBot::showGamesMenu(int64_t chat_id) {
    string games_text = "🎮 *ВЫБОР ИГРЫ*\n\n"
                       //надо понять, что за игры точно будут, поэтому пока
                       //ничего писать не буду
    
    sendMessage(chat_id, games_text, Keyboard::createGamesMenu());
    user_states_[chat_id] = "choose_game";
}

void TelegramBot::showBetsMenu(int64_t chat_id) {
    string bets_text = "💰 *ВЫБОР СТАВКИ*\n\n"
                      "Выберите размер ставки:\n\n"
                      "• *10* - минимальная ставка\n"
                      "• *50* - небольшая ставка\n"
                      "• *100* - средняя ставка\n"
                      "• *500* - высокая ставка\n"
                      "• *1000* - очень высокая ставка\n"
                      "• *5000* - максимальная ставка\n"
                      "• *ВСЯ СТАВКА* - поставить всё\n"
                      "• *ИЗМЕНИТЬ СТАВКУ* - ввести свою"; //тут под вопросом
    
    sendMessage(chat_id, bets_text, Keyboard::createBetsMenu());
    user_states_[chat_id] = "choose_bet";
}

void TelegramBot::handleCommand(int64_t chat_id, const string& command, const string& username) {
    if (command == "/start") {
        showMainMenu(chat_id, username);
    }
    else if (command == "/help") {
        string help_text = "*ПОМОЩЬ*\n\n"
                          "*Как пользоваться ботом:*\n\n"
                          "1. Начните с команды /start\n"
                          "2. Выберите игру из меню \n"
                          "3. Выберите агента \n"
                          "4. Установите ставку \n"
                          "5. Начните игру!\n\n"
                          "*Команды:*\n"
                          "/start - начать\n"
                          "/help - помощь\n"
                          "/menu - главное меню";
        
        sendMessage(chat_id, help_text, Keyboard::createMainMenu());
    }
    else if (command == "/menu") {
        showMainMenu(chat_id, username);
    }
}

void TelegramBot::handleMessage(int64_t chat_id, const string& text, const string& username) {
    cout << "Получено сообщение от " << username << ": " << text << endl;

    if (text.rfind("/", 0) == 0) {
        handleCommand(chat_id, text, username);
        return;
    }

    if (text == "Выбрать игру") {
        showGamesMenu(chat_id);
    }
    else if (text == "Выбрать агента") {
        showAgentsMenu(chat_id);
    }
    else if (text == "Сделать ставку") {
        showBetsMenu(chat_id);
    }
    /*
    else if (text == "Статистика") {
        string stats_text = "*ВАША СТАТИСТИКА*\n\n"
                           "*Игр сыграно:* 0\n"
                           "*Выиграно:* 0\n"
                           "*Проиграно:* 0\n"
                           "*Винрейт:* 0%\n"
                           "*Лучшая игра:* -";
        
        sendMessage(chat_id, stats_text, Keyboard::createMainMenu());
    }
    else if (text == "Баланс") {
        string balance_text = "*ВАШ БАЛАНС*\n\n"
                             "*Текущий баланс:* 1000\n"
                             "*Текущая ставка:* не установлена\n"
                             "*Текущий агент:* не выбран\n"
                             "*Текущая игра:* не выбрана";
        
        sendMessage(chat_id, balance_text, Keyboard::createMainMenu());
    }
    else if (text == "Помощь") {
        handleCommand(chat_id, "/help", username);
    }*/
    
    // Обработка кнопок выбора агента
    
    
    // Обработка кнопок выбора игры
    
    
    // Обработка кнопок ставок
    
    // Обработка ввода суммы ставки
    else if (user_states_[chat_id] == "enter_bet") {
        try {
            int bet = stoi(text);
            if (bet >= 10 && bet <= 5000) {
                string bet_response = "*СТАВКА УСТАНОВЛЕНА!*\n\n"
                                     "Размер ставки: " + to_string(bet) + "\n\n"
                                     "Теперь можно начинать игру!";
                
                sendMessage(chat_id, bet_response, Keyboard::createMainMenu());
                user_states_[chat_id] = "main_menu";
            }
            else {
                sendMessage(chat_id, "*ОШИБКА*\nСтавка должна быть от 10 до 5000!", Keyboard::createBetsMenu());
            }
        }
        catch (...) {
            sendMessage(chat_id, "*ОШИБКА*\nВведите число от 10 до 5000!", Keyboard::createBetsMenu());
        }
    }

    else if (text == "Назад в меню") {
        showMainMenu(chat_id, username);
    }
}

void TelegramBot::run() {
    cout << "Бот запущен! Ожидание сообщений..." << endl;
    
    int64_t last_update_id = 0;
    
    while (true) {
        string response = makeRequest("getUpdates", "offset=" + to_string(last_update_id + 1) + "&timeout=60");
        
        Json::Value root;
        Json::CharReaderBuilder reader;
        string errors;
        istringstream response_stream(response);
        
        if (Json::parseFromStream(reader, response_stream, &root, &errors)) {
            if (root["ok"].asBool()) {
                const Json::Value& updates = root["result"];
                
                for (const auto& update : updates) {
                    last_update_id = update["update_id"].asInt64();
                    
                    if (update.isMember("message")) {
                        const Json::Value& message = update["message"];
                        int64_t chat_id = message["chat"]["id"].asInt64();
                        string text = message["text"].asString();
                        
                        string username = "игрок";
                        if (message["chat"].isMember("username")) {
                            username = message["chat"]["username"].asString();
                        }
                        else if (message["chat"].isMember("first_name")) {
                            username = message["chat"]["first_name"].asString();
                        }
                        
                        handleMessage(chat_id, text, username);
                    }
                }
            }
        }
        
        // Пауза между запросами
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
    }
}