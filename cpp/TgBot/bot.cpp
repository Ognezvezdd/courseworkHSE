#include "bot.hpp"
#include "game_manager.hpp"
#include "keyboard.hpp"
#include <algorithm>
#include <ctime>
#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

TelegramBot::TelegramBot(const string &token, GameManager &game_manager)
    : token_(token), game_manager_(game_manager) {
  base_url_ = "https://api.telegram.org/bot" + token + "/";
  curl_global_init(CURL_GLOBAL_DEFAULT);
  cout << "🤖 Бот инициализирован. URL: " << base_url_ << endl;
}

TelegramBot::~TelegramBot() { curl_global_cleanup(); }

string TelegramBot::makeRequest(const string &method, const string &params) {
  CURL *curl = curl_easy_init();
  string response;

  if (!curl) {
    cerr << "❌ Ошибка инициализации CURL" << endl;
    return "";
  }

  string url = base_url_ + method;
  cout << "🌐 Запрос: " << method << endl;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  struct curl_slist *headers = NULL;
  if (!params.empty()) {
    cout << "📦 Параметры: " << params << endl;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    cerr << "❌ Ошибка CURL: " << curl_easy_strerror(res) << endl;
  } else {
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    cout << "📡 HTTP код: " << http_code << endl;
    cout << "✅ Ответ: "
         << (response.empty() ? "пусто" : response.substr(0, 200)) << endl;
  }

  if (headers) {
    curl_slist_free_all(headers);
  }

  curl_easy_cleanup(curl);
  return response;
}

void TelegramBot::sendMessage(int64_t chat_id, const string &text,
                              const string &reply_markup, bool markdown) {
  cout << "📤 Отправка в чат " << chat_id << ": " << text.substr(0, 50) << "..."
       << endl;

  Json::Value params;
  params["chat_id"] = chat_id;
  params["text"] = text;

  if (markdown) {
    params["parse_mode"] = "Markdown";
  }

  if (!reply_markup.empty()) {
    Json::Value markup;
    Json::CharReaderBuilder reader;
    string errors;
    istringstream markup_stream(reply_markup);

    if (Json::parseFromStream(reader, markup_stream, &markup, &errors)) {
      params["reply_markup"] = markup;
    } else {
      cerr << "❌ Ошибка парсинга reply_markup: " << errors << endl;
    }
  }

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  string json_params = Json::writeString(writer, params);

  string response = makeRequest("sendMessage", json_params);

  // Проверяем ответ
  if (!response.empty()) {
    Json::Value root;
    Json::CharReaderBuilder reader;
    string errors;
    istringstream response_stream(response);

    if (Json::parseFromStream(reader, response_stream, &root, &errors)) {
      if (!root["ok"].asBool()) {
        cerr << "❌ Ошибка отправки: " << root["description"].asString()
             << endl;
      } else {
        cout << "✅ Сообщение отправлено успешно!" << endl;
      }
    }
  }
}

void TelegramBot::showMainMenu(int64_t chat_id) {
  Keyboard keyboard;
  string menu_text = "🎮 *Главное меню*\n\n"
                     "1. Выберите игру\n"
                     "2. Выберите агента\n"
                     "3. Сделайте ставку\n"
                     "4. Начните игру";

  sendMessage(chat_id, menu_text, keyboard.createMainMenu(), true);
}

void TelegramBot::showAgentsMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id, "🤖 Выберите агента:", keyboard.createAgentsMenu());
}

void TelegramBot::showGamesMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id, "🎲 Выберите игру:", keyboard.createGamesMenu());
}

void TelegramBot::showBetsMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id, "💰 Выберите ставку:", keyboard.createBetsMenu());
}

void TelegramBot::showPlayMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id, "🎯 Выберите режим игры:", keyboard.createPlayMenu());
}

void TelegramBot::handleMessage(int64_t chat_id, const string &text,
                                const string &username) {
  cout << "📥 [" << username << "]: " << text << endl;

  if (user_states_.find(chat_id) == user_states_.end()) {
    user_states_[chat_id] = UserState();
  }
  UserState &state = user_states_[chat_id];

  // Обработка команд
  if (text == "/start" || text == "Назад в меню" || text == "/menu") {
    showMainMenu(chat_id);
  } else if (text == "Выбрать игру") {
    showGamesMenu(chat_id);
  } else if (text == "Выбрать агента") {
    showAgentsMenu(chat_id);
  } else if (text == "Сделать ставку") {
    showBetsMenu(chat_id);
  } else if (text == "Начать игру") {
    if (state.selected_agent.empty()) {
      sendMessage(chat_id,
                  "⚠️ Вы не выбрали агента! Пожалуйста, выберите агента.",
                  Keyboard().createAgentsMenu());
      return;
    }
    if (state.bet_amount <= 0) {
      sendMessage(chat_id,
                  "⚠️ Вы не сделали ставку! Пожалуйста, сделайте ставку.",
                  Keyboard().createBetsMenu());
      return;
    }
    showPlayMenu(chat_id);
  } else if (text == "Крестики-нолики 5x5") {
    state.selected_game = "tic_tac_toe_5x5";
    sendMessage(chat_id, "✅ Выбрана игра: Крестики-нолики 5x5",
                Keyboard().createMainMenu());
  } else if (text == "Random (случайный)") {
    state.selected_agent = "random";
    sendMessage(chat_id, "✅ Выбран ваш агент: Random",
                Keyboard().createMainMenu());
  } else if (text == "Heuristic (умный)") {
    state.selected_agent = "heuristic";
    sendMessage(chat_id, "✅ Выбран ваш агент: Heuristic",
                Keyboard().createMainMenu());
  } else if (text == "QLearning (обучаемый)") {
    state.selected_agent = "qlearning";
    sendMessage(chat_id, "✅ Выбран ваш агент: QLearning",
                Keyboard().createMainMenu());
  } else if (text == "10" || text == "50" || text == "100" || text == "500" ||
             text == "1000") {
    state.bet_amount = stoi(text);
    sendMessage(chat_id, "✅ Установлена ставка: " + text + " очков",
                Keyboard().createMainMenu());
  } else if (text == "Случайный противник") {
    state.opponent_agent = "random";
    sendMessage(chat_id,
                "🎲 Запуск игры: " + state.selected_agent + " vs Random...");
    GameResult result = game_manager_.runGame(state.selected_agent, "random",
                                              (int)time(nullptr));
    string msg = "🏁 Победитель: " + result.winner +
                 "\nШагов: " + to_string(result.steps);
    sendMessage(chat_id, msg, Keyboard().createMainMenu());
  } else if (text == "Против Heuristic") {
    state.opponent_agent = "heuristic";
    sendMessage(chat_id,
                "🧠 Запуск игры: " + state.selected_agent + " vs Heuristic...");
    GameResult result = game_manager_.runGame(state.selected_agent, "heuristic",
                                              (int)time(nullptr));
    string msg = "🏁 Победитель: " + result.winner +
                 "\nШагов: " + to_string(result.steps);
    sendMessage(chat_id, msg, Keyboard().createMainMenu());
  } else if (text == "Против QLearning") {
    state.opponent_agent = "qlearning";
    sendMessage(chat_id,
                "🤖 Запуск игры: " + state.selected_agent + " vs QLearning...");
    GameResult result = game_manager_.runGame(state.selected_agent, "qlearning",
                                              (int)time(nullptr));
    string msg = "🏁 Победитель: " + result.winner +
                 "\nШагов: " + to_string(result.steps);
    sendMessage(chat_id, msg, Keyboard().createMainMenu());
  } else {
    sendMessage(chat_id, "🤔 Неизвестная команда. Используйте меню:",
                Keyboard().createMainMenu());
  }
}

void TelegramBot::run() {
  cout << "🚀 Запуск бота..." << endl;
  cout << "🔧 Используется токен: " << token_.substr(0, 10) << "..." << endl;

  int64_t last_update_id = 0;

  // Сначала делаем тестовый запрос для проверки бота
  cout << "🔄 Проверка соединения..." << endl;
  string test_response = makeRequest("getMe", "");

  if (!test_response.empty()) {
    Json::Value root;
    Json::CharReaderBuilder reader;
    string errors;
    istringstream test_stream(test_response);

    if (Json::parseFromStream(reader, test_stream, &root, &errors)) {
      if (root["ok"].asBool()) {
        cout << "✅ Бот активен: " << root["result"]["username"].asString()
             << endl;
      } else {
        cerr << "❌ Ошибка бота: " << root["description"].asString() << endl;
        return;
      }
    }
  }

  cout << "🤖 Бот готов к работе! Ожидание сообщений..." << endl;

  while (true) {
    try {
      // Запрашиваем обновления
      Json::Value request_params;
      request_params["offset"] = last_update_id + 1;
      request_params["timeout"] = 30;
      request_params["limit"] = 10;

      Json::StreamWriterBuilder writer;
      writer["indentation"] = "";
      string json_request = Json::writeString(writer, request_params);

      cout << "🔄 Получение обновлений (offset: " << last_update_id + 1
           << ")..." << endl;
      string response = makeRequest("getUpdates", json_request);

      if (response.empty()) {
        cerr << "⚠️ Пустой ответ от сервера" << endl;
#ifdef _WIN32
        Sleep(2000);
#else
        sleep(2);
#endif
        continue;
      }

      Json::Value root;
      Json::CharReaderBuilder reader;
      string errors;
      istringstream response_stream(response);

      if (!Json::parseFromStream(reader, response_stream, &root, &errors)) {
        cerr << "❌ Ошибка парсинга JSON: " << errors << endl;
        cerr << "📄 Ответ был: " << response << endl;
#ifdef _WIN32
        Sleep(3000);
#else
        sleep(3);
#endif
        continue;
      }

      if (!root["ok"].asBool()) {
        cerr << "❌ Ошибка Telegram API: " << root["description"].asString()
             << endl;
#ifdef _WIN32
        Sleep(3000);
#else
        sleep(3);
#endif
        continue;
      }

      const Json::Value &updates = root["result"];

      if (updates.size() > 0) {
        cout << "📥 Получено " << updates.size() << " сообщение(ий)" << endl;
      }

      for (const auto &update : updates) {
        int64_t update_id = update["update_id"].asInt64();
        last_update_id = update_id;

        cout << "🆔 Update ID: " << update_id << endl;

        if (update.isMember("message")) {
          const Json::Value &message = update["message"];

          if (!message.isMember("text")) {
            cout << "⚠️ Сообщение без текста" << endl;
            continue;
          }

          int64_t chat_id = message["chat"]["id"].asInt64();
          string text = message["text"].asString();

          string username = "игрок";
          if (message["chat"].isMember("username")) {
            username = message["chat"]["username"].asString();
          } else if (message["chat"].isMember("first_name")) {
            username = message["chat"]["first_name"].asString();
          }

          cout << "👤 Чат ID: " << chat_id << ", Пользователь: " << username
               << endl;

          handleMessage(chat_id, text, username);
        } else if (update.isMember("callback_query")) {
          cout << "🔘 Callback query получен" << endl;
          // Обработка callback query (если нужно)
        }
      }

    } catch (const exception &e) {
      cerr << "❌ Исключение: " << e.what() << endl;
    }

// Пауза между запросами
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
  }
}