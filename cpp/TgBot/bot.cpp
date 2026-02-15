#include "bot.hpp"
#include "game_manager.hpp"
#include "keyboard.hpp"
#include <algorithm>
#include <ctime>
#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <random>
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

  makeRequest("sendMessage", json_params);
}

void TelegramBot::sendPhoto(int64_t chat_id, const string &photo_url,
                            const string &caption) {
  cout << "🖼️ Отправка фото в чат " << chat_id << ": " << photo_url << endl;

  Json::Value params;
  params["chat_id"] = chat_id;
  params["photo"] = photo_url;
  if (!caption.empty()) {
    params["caption"] = caption;
    params["parse_mode"] = "Markdown";
  }

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  string json_params = Json::writeString(writer, params);

  makeRequest("sendPhoto", json_params);
}

void TelegramBot::uploadPhoto(int64_t chat_id, const string &file_path,
                              const string &caption) {
  cout << "📤 Загрузка фото в чат " << chat_id << ": " << file_path << endl;

  CURL *curl = curl_easy_init();
  if (!curl) {
    cerr << "❌ Ошибка инициализации CURL для загрузки фото" << endl;
    return;
  }

  string url = base_url_ + "sendPhoto";

  curl_mime *mime = curl_mime_init(curl);
  curl_mimepart *part;

  part = curl_mime_addpart(mime);
  curl_mime_name(part, "chat_id");
  curl_mime_data(part, to_string(chat_id).c_str(), CURL_ZERO_TERMINATED);

  part = curl_mime_addpart(mime);
  curl_mime_name(part, "photo");
  curl_mime_filedata(part, file_path.c_str());

  if (!caption.empty()) {
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "caption");
    curl_mime_data(part, caption.c_str(), CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "parse_mode");
    curl_mime_data(part, "Markdown", CURL_ZERO_TERMINATED);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  string response;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    cerr << "❌ Ошибка CURL при загрузке фото: " << curl_easy_strerror(res)
         << endl;
  }

  curl_mime_free(mime);
  curl_easy_cleanup(curl);
}

void TelegramBot::showMainMenu(int64_t chat_id) {
  Keyboard keyboard;
  string menu_text = "🎮 *Главное меню*\n\n"
                     "Выберите игру:";

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

void TelegramBot::showMafiaMenu(int64_t chat_id) {
  Keyboard keyboard;
  string menu_text = "🎭 *Мафия*\n\n"
                     "Классическая социальная игра.\n"
                     "Агенты делятся на мафию и мирных жителей.";

  sendMessage(chat_id, menu_text, keyboard.createMafiaMenu(), true);
}

void TelegramBot::showMafiaAgentsMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id, "👥 Выберите количество игроков (6-12):",
              keyboard.createMafiaAgentsMenu());
}

void TelegramBot::showMafiaSettingsMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id,
              "⚙️ Настройки игры в мафию:", keyboard.createMafiaSettingsMenu());
}

void TelegramBot::showMafiaPlayMenu(int64_t chat_id) {
  Keyboard keyboard;
  sendMessage(chat_id, "🎭 Настройки игры:", keyboard.createMafiaPlayMenu());
}

void TelegramBot::handleTicTacToeGame(int64_t chat_id, UserState &state) {
  if (state.selected_agent.empty()) {
    sendMessage(chat_id, "⚠️ Сначала выберите агента!",
                Keyboard().createAgentsMenu());
    return;
  }

  if (state.bet_amount <= 0) {
    sendMessage(chat_id, "⚠️ Сначала сделайте ставку!",
                Keyboard().createBetsMenu());
    return;
  }

  sendMessage(chat_id, "🎲 Запуск игры: " + state.selected_agent + " vs " +
                           state.opponent_agent + "...");

  GameResult result = game_manager_.runGame(
      state.selected_agent, state.opponent_agent, (int)time(nullptr));
  result.bet_amount = state.bet_amount;

  stringstream ss;
  ss << "🏁 *Игра завершена!*\n\n";
  ss << "👤 Ваш агент (X): " << state.selected_agent << "\n";
  ss << "🤖 Противник (O): " << state.opponent_agent << "\n";
  ss << "🏆 Победитель: *" << result.winner << "*\n";
  ss << "⏱ Шагов: " << result.steps << "\n\n";

  if (result.winner == "X") {
    ss << "💰 *ВЫ ВЫИГРАЛИ!* 🚀\n";
    ss << "Вы получаете: " << (state.bet_amount * 2) << " очков";
  } else if (result.winner == "O") {
    ss << "💸 *ВЫ ПРОИГРАЛИ* 😢\n";
    ss << "Удачи в следующий раз!";
  } else {
    ss << "🤝 *НИЧЬЯ*\n";
    ss << "Ставка " << state.bet_amount << " возвращена";
  }

  if (!result.image_filename.empty()) {
    string local_path = "output/" + result.image_filename;
    if (access(local_path.c_str(), F_OK) == 0) {
      uploadPhoto(chat_id, local_path, ss.str());
    } else if (access(("/app/" + local_path).c_str(), F_OK) == 0) {
      uploadPhoto(chat_id, "/app/" + local_path, ss.str());
    } else if (!result.image_url.empty()) {
      sendPhoto(chat_id, result.image_url, ss.str());
    } else {
      sendMessage(chat_id, ss.str(), Keyboard().createMainMenu(), true);
    }
  } else if (!result.image_url.empty()) {
    sendPhoto(chat_id, result.image_url, ss.str());
  } else {
    sendMessage(chat_id, ss.str(), Keyboard().createMainMenu(), true);
  }
}

void TelegramBot::handleMafiaGame(int64_t chat_id, UserState &state) {
  cout << "🎭 Attempting to start Mafia game. Bet: " << state.bet_amount
       << ", Players: " << state.mafia_players << endl;

  if (state.bet_amount <= 0) {
    sendMessage(chat_id, "⚠️ Сначала сделайте ставку для игры в мафию!",
                Keyboard::createBetsMenu());
    return;
  }

  sendMessage(chat_id, "🎭 Запускаю игру в мафию с " +
                           to_string(state.mafia_players) +
                           " агентами...\n"
                           "Это может занять несколько минут...");

  try {
    auto agents = game_manager_.getAvailableMafiaAgents();
    if (agents.empty()) {
      sendMessage(chat_id, "❌ Ошибка: список агентов пуст! Попробуйте позже.",
                  Keyboard::createMafiaMenu());
      return;
    }

    MafiaGameResult result = game_manager_.runMafiaGame(
        agents, state.mafia_players, state.bet_amount, true);

    formatMafiaResult(chat_id, result, state.bet_amount);
  } catch (const std::exception &e) {
    cerr << "❌ Exception in handleMafiaGame: " << e.what() << endl;
    sendMessage(chat_id,
                "❌ Произошла ошибка при запуске игры: " + string(e.what()),
                Keyboard::createMafiaMenu());
  } catch (...) {
    cerr << "❌ Unknown exception in handleMafiaGame" << endl;
    sendMessage(chat_id, "❌ Произошла критическая ошибка при запуске игры.",
                Keyboard::createMafiaMenu());
  }
}

void TelegramBot::formatMafiaResult(int64_t chat_id,
                                    const MafiaGameResult &result,
                                    int bet_amount) {
  stringstream ss;
  ss << "🎭 *Игра в мафию завершена!*\n\n";
  ss << "🏆 Победитель: *";

  if (result.winner == "citizens") {
    ss << "МИРНЫЕ ЖИТЕЛИ* 🎉\n";
    ss << "💰 Выигрыш: " << result.win_amount << " очков (ставка x3)\n";
  } else if (result.winner == "mafia") {
    ss << "МАФИЯ* 💀\n";
    ss << "😢 Ставка проиграна\n";
  } else {
    ss << "ОШИБКА*\n";
  }

  ss << "\n📊 Статистика:\n";
  ss << "• Дней игры: " << result.total_days << "\n";
  ss << "• Выжило игроков: " << result.surviving_players << "\n";
  ss << "• Всего игроков: "
     << (result.mafia_team.size() + result.citizen_team.size()) << "\n";

  ss << "\n👥 Состав мафии:\n";
  for (const auto &mafia : result.mafia_team) {
    ss << "• " << mafia << "\n";
  }

  ss << "\n👥 Мирные жители:\n";
  for (const auto &citizen : result.citizen_team) {
    ss << "• " << citizen << "\n";
  }

  if (!result.killed_players.empty()) {
    ss << "\n⚰️ Убитые игроки:\n";
    for (const auto &killed : result.killed_players) {
      if (ss.str().length() + killed.length() < 4000) {
        ss << "• " << killed << "\n";
      }
    }
  }

  // Отправляем результат
  sendMessage(chat_id, ss.str(), Keyboard().createMainMenu(), true);

  // Отправляем лог чата, если есть
  if (!result.chat_log.empty()) {
    sendMafiaChatLog(chat_id, result.chat_log);
  }

  // Отправляем изображение, если есть
  if (!result.image_url.empty()) {
    sendPhoto(chat_id, result.image_url, "🎭 Визуализация игры в мафию");
  }
}

void TelegramBot::sendMafiaChatLog(
    int64_t chat_id, const std::vector<Mafia::ChatMessage> &chat_log) {
  stringstream ss;
  ss << "💬 *Лог чата мафии:*\n\n";

  int count = 0;
  for (const auto &msg : chat_log) {
    if (msg.is_public && count < 20) { // Ограничиваем количество сообщений
      string time_of_day = msg.is_night ? "🌙" : "☀️";
      ss << time_of_day << " " << msg.player_name << ": " << msg.text << "\n";
      count++;
    }
  }

  if (count > 0) {
    sendMessage(chat_id, ss.str(), "", true);
  }
}

void TelegramBot::handleMessage(int64_t chat_id, const string &text,
                                const string &username) {
  cout << "📥 [" << username << "]: " << text << endl;

  if (user_states_.find(chat_id) == user_states_.end()) {
    user_states_[chat_id] = UserState();
  }
  UserState &state = user_states_[chat_id];

  // Основные команды
  if (text == "/start" || text == "Назад в главное меню" || text == "/menu") {
    showMainMenu(chat_id);
  }
  // Главное меню
  else if (text == "🎮 Крестики-нолики") {
    state.game_mode = "tic_tac_toe";
    showGamesMenu(chat_id);
  } else if (text == "🎭 Мафия") {
    state.game_mode = "mafia";
    showMafiaMenu(chat_id);
  }
  // Меню крестиков-ноликов
  else if (text == "Крестики-нолики 5x5") {
    state.selected_game = "tic_tac_toe_5x5";
    sendMessage(chat_id,
                "✅ Выбрана игра: Крестики-нолики 5x5\n"
                "Теперь выберите агента и сделайте ставку.",
                Keyboard().createMainMenu());
  } else if (text == "🎭 Перейти к мафии") {
    state.game_mode = "mafia";
    showMafiaMenu(chat_id);
  }
  // Выбор агентов
  else if (text == "Random (случайный)") {
    state.selected_agent = "random";
    sendMessage(chat_id, "✅ Выбран агент: Random",
                Keyboard().createMainMenu());
  } else if (text == "Heuristic (умный)") {
    state.selected_agent = "heuristic";
    sendMessage(chat_id, "✅ Выбран агент: Heuristic",
                Keyboard().createMainMenu());
  } else if (text == "QLearning (обучаемый)") {
    state.selected_agent = "qlearning";
    sendMessage(chat_id, "✅ Выбран агент: QLearning",
                Keyboard().createMainMenu());
  }
  // Ставки
  else if (text == "10" || text == "50" || text == "100" || text == "500" ||
           text == "1000" || text == "5000") {
    state.bet_amount = stoi(text);
    string response = "✅ Установлена ставка: " + text + " очков";

    if (state.game_mode == "mafia") {
      sendMessage(chat_id, response, Keyboard::createMafiaMenu());
    } else {
      sendMessage(chat_id, response, Keyboard::createGamesMenu());
    }
  }
  // Оппоненты для крестиков-ноликов
  else if (text == "Случайный противник") {
    state.opponent_agent = "random";
    handleTicTacToeGame(chat_id, state);
  } else if (text == "Против Heuristic") {
    state.opponent_agent = "heuristic";
    handleTicTacToeGame(chat_id, state);
  } else if (text == "Против QLearning") {
    state.opponent_agent = "qlearning";
    handleTicTacToeGame(chat_id, state);
  }
  // Меню мафии
  else if (text == "🎭 Начать игру в мафию") {
    showMafiaPlayMenu(chat_id);
  } else if (text == "👥 Выбрать количество игроков") {
    showMafiaAgentsMenu(chat_id);
  } else if (text == "⚙️ Настройки мафии") {
    showMafiaSettingsMenu(chat_id);
  } else if (text == "📋 Правила мафии") {
    string rules = "🎭 *Правила Мафии:*\n\n"
                   "• Играют 6-12 агентов\n"
                   "• Роли: Мафия, Дон, Шериф, Доктор, Мирные\n"
                   "• Ночью мафия убивает, шериф проверяет, доктор лечит\n"
                   "• Днем игроки обсуждают и голосуют за изгнание\n"
                   "• Мафия побеждает, когда их больше или столько же\n"
                   "• Мирные побеждают, когда вся мафия изгнана\n\n"
                   "💰 *Ставки:* x3 за победу мирных";
    sendMessage(chat_id, rules, Keyboard().createMafiaMenu(), true);
  }
  // Выбор количества игроков для мафии
  else if (text == "6 игроков" || text == "7 игроков" || text == "8 игроков" ||
           text == "9 игроков" || text == "10 игроков" ||
           text == "12 игроков") {
    state.mafia_players = stoi(text.substr(0, text.find(' ')));
    sendMessage(chat_id, "✅ Выбрано " + text + " для игры в мафию",
                Keyboard().createMafiaMenu());
  } else if (text == "Случайное количество") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(6, 12);
    state.mafia_players = dis(gen);
    sendMessage(chat_id,
                "✅ Случайно выбрано " + to_string(state.mafia_players) +
                    " игроков",
                Keyboard().createMafiaMenu());
  }
  // Настройки мафии
  else if (text == "▶️ Начать игру сейчас") {
    handleMafiaGame(chat_id, state);
  } else if (text == "👥 Выбрать своих агентов") {
    sendMessage(chat_id, "⏳ Эта функция в разработке...",
                Keyboard().createMafiaMenu());
  } else if (text == "💰 Сделать ставку") {
    showBetsMenu(chat_id);
  }
  // Назад
  else if (text == "Назад в меню мафии") {
    showMafiaMenu(chat_id);
  } else if (text == "Назад в меню") {
    if (state.game_mode == "mafia") {
      showMafiaMenu(chat_id);
    } else {
      showMainMenu(chat_id);
    }
  }
  // Неизвестная команда
  else {
    sendMessage(chat_id, "🤔 Неизвестная команда. Используйте меню:",
                Keyboard().createMainMenu());
  }
}

void TelegramBot::run() {
  cout << "🚀 Запуск бота..." << endl;
  cout << "🔧 Используется токен: " << token_.substr(0, 10) << "..." << endl;

  int64_t last_update_id = 0;

  // Проверка соединения
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
      Json::Value request_params;
      request_params["offset"] = last_update_id + 1;
      request_params["timeout"] = 30;
      request_params["limit"] = 10;

      Json::StreamWriterBuilder writer;
      writer["indentation"] = "";
      string json_request = Json::writeString(writer, request_params);

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

      for (const auto &update : updates) {
        int64_t update_id = update["update_id"].asInt64();
        last_update_id = update_id;

        if (update.isMember("message")) {
          const Json::Value &message = update["message"];

          if (!message.isMember("text")) {
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

          handleMessage(chat_id, text, username);
        }
      }

    } catch (const exception &e) {
      cerr << "❌ Исключение: " << e.what() << endl;
    }

#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
  }
}