#include "bot.hpp"
#include "keyboard.hpp"
#include "game_manager.hpp"
#include <curl/curl.h>
#include <json/json.h>
#include <iostream>
#include <sstream>
#include <algorithm>  // ДОБАВИТЬ ЭТУ СТРОКУ!
#include <vector>     // ДОБАВИТЬ ЭТУ СТРОКУ!

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

TelegramBot::TelegramBot(const string& token, GameManager& game_manager) 
    : token_(token), game_manager_(game_manager) {
    base_url_ = "https://api.telegram.org/bot" + token + "/";
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

TelegramBot::~TelegramBot() {
    curl_global_cleanup();
}

string TelegramBot::makeRequest(const string& method, const string& params) {
    CURL* curl = curl_easy_init();
    string response;
    
    if (!curl) {
        return "";
    }
    
    string url = base_url_ + method;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    struct curl_slist* headers = NULL;
    if (!params.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);  // Можно использовать (void)res; чтобы убрать warning
    
    if (headers) {
        curl_slist_free_all(headers);
    }
    
    curl_easy_cleanup(curl);
    return response;
}

void TelegramBot::sendMessage(int64_t chat_id, const string& text, const string& reply_markup, bool markdown) {
    cout << "Отправка сообщения: " << text.substr(0, 50) << "..." << endl;
    
    Json::Value params;
    params["chat_id"] = chat_id;
    params["text"] = text;
    
    if (!reply_markup.empty()) {
        Json::Value markup;
        Json::Reader reader;
        if (reader.parse(reply_markup, markup)) {
            params["reply_markup"] = markup;
        }
    }
    
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    string json_params = Json::writeString(writer, params);
    
    makeRequest("sendMessage", json_params);
}

// ... остальные функции (showMainMenu, showAgentsMenu и т.д.) без изменений ...

void TelegramBot::handleMessage(int64_t chat_id, const string& text, const string& username) {
    cout << "Получено сообщение от " << username << ": " << text << endl;

    // УБИРАЕМ проблемный код с find() или исправляем его:
    static const vector<string> button_texts = {
        "Выбрать игру", "Выбрать агента", "Сделать ставку", 
        "Начать игру", "Назад в меню", "Крестики-нолики 5x5",
        "Random (случайный)", "Heuristic (умный)", "QLearning (обучаемый)",
        "10", "50", "100", "500", "1000"
    };
    
    // Простая проверка - это текст от кнопки?
    bool is_button_text = false;
    for (const auto& button_text : button_texts) {
        if (text == button_text) {
            is_button_text = true;
            break;
        }
    }
    
    if (is_button_text) {
        cout << "Нажата кнопка: " << text << endl;
    }
    
    // ... остальной код handleMessage без изменений ...
}

void TelegramBot::run() {
    cout << "✅ Бот запускается..." << endl;
    
    // Начинаем с последнего известного update_id
    int64_t last_update_id = 252622674;  // Установите найденное значение!
    
    cout << "🔄 Начинаем с update_id: " << last_update_id << endl;
    cout << "🤖 Бот готов к работе! Ожидание сообщений..." << endl;
    
    while (true) {
        try {
            // Запрашиваем только НОВЫЕ сообщения
            string request = "{\"offset\":" + to_string(last_update_id + 1) + 
                           ",\"timeout\":30,\"limit\":10}";
            
            string response = makeRequest("getUpdates", request);
            
            Json::Value root;
            Json::CharReaderBuilder reader;
            string errors;
            istringstream response_stream(response);
            
            if (!Json::parseFromStream(reader, response_stream, &root, &errors)) {
                cerr << "❌ Ошибка парсинга JSON: " << errors << endl;
                #ifdef _WIN32
                    Sleep(2000);
                #else
                    sleep(2);
                #endif
                continue;
            }
            
            if (!root["ok"].asBool()) {
                cerr << "❌ Ошибка Telegram API" << endl;
                #ifdef _WIN32
                    Sleep(2000);
                #else
                    sleep(2);
                #endif
                continue;
            }
            
            const Json::Value& updates = root["result"];  // ДОБАВЬТЕ 'const'
            
            if (updates.size() > 0) {
                cout << "📥 Получено " << updates.size() << " сообщение(ий)" << endl;
            }
            
            // Обрабатываем каждое сообщение
            for (const auto& update : updates) {
                int64_t update_id = update["update_id"].asInt64();
                
                // ВАЖНО: обновляем last_update_id
                last_update_id = update_id;
                
                if (update.isMember("message")) {
                    const Json::Value& message = update["message"];
                    
                    if (!message.isMember("text")) {
                        continue;
                    }
                    
                    int64_t chat_id = message["chat"]["id"].asInt64();
                    string text = message["text"].asString();
                    
                    string username = "игрок";
                    if (message["chat"].isMember("username")) {
                        username = message["chat"]["username"].asString();
                    }
                    else if (message["chat"].isMember("first_name")) {
                        username = message["chat"]["first_name"].asString();
                    }
                    
                    cout << "👤 [" << username << "]: " << text << " (update_id: " << update_id << ")" << endl;
                    
                    handleMessage(chat_id, text, username);
                }
            }
            
        } catch (const exception& e) {
            cerr << "❌ Исключение: " << e.what() << endl;
        }
        
        // Пауза
        #ifdef _WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
    }
}