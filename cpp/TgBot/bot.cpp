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
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 45L);

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
    }
  }

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  string json_params = Json::writeString(writer, params);

  makeRequest("sendMessage", json_params);
}

void TelegramBot::sendPhoto(int64_t chat_id, const string &photo_url,
                            const string &caption) {
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
  CURL *curl = curl_easy_init();
  if (!curl)
    return;

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

  curl_easy_perform(curl);

  curl_mime_free(mime);
  curl_easy_cleanup(curl);
}

// ============================================================
//  Показ меню
// ============================================================

void TelegramBot::showMainMenu(int64_t chat_id) {
  sendMessage(chat_id,
              "🎮 *Главное меню*\n\nВыберите игру:", Keyboard::createMainMenu(),
              true);
}

void TelegramBot::showTicTacToeMenu(int64_t chat_id) {
  sendMessage(chat_id,
              "🎮 *Крестики-нолики 5×5*\n\n"
              "Настройте параметры и нажмите «Запустить игру».",
              Keyboard::createTicTacToeMenu(), true);
}

void TelegramBot::showMafiaMenu(int64_t chat_id) {
  sendMessage(chat_id,
              "🎭 *Мафия*\n\n"
              "Настройте параметры и нажмите «Запустить мафию».",
              Keyboard::createMafiaMenu(), true);
}

void TelegramBot::showBunkerMenu(int64_t chat_id) {
  sendMessage(chat_id,
              "🛡️ *Бункер*\n\n"
              "Настройте вместимость, ставку и нажмите «Запустить бункер».",
              Keyboard::createBunkerMenu(), true);
}

// ============================================================
//  Игра Крестики-нолики
// ============================================================

void TelegramBot::handleTicTacToeGame(int64_t chat_id, UserState &state) {
  if (state.selected_agent.empty()) {
    sendMessage(chat_id, "⚠️ Сначала выберите агента!",
                Keyboard::createTicTacToeMenu());
    return;
  }
  if (state.opponent_agent.empty()) {
    sendMessage(chat_id, "⚠️ Сначала выберите противника!",
                Keyboard::createTicTacToeMenu());
    return;
  }

  sendMessage(chat_id, "🎲 Запуск: " + state.selected_agent + " vs " +
                           state.opponent_agent + "...");

  GameResult result = game_manager_.runGame(
      state.selected_agent, state.opponent_agent, (int)time(nullptr));

  stringstream ss;
  ss << "🏁 *Игра завершена!*\n\n";
  ss << "👤 Ваш агент (X): " << state.selected_agent << "\n";
  ss << "🤖 Противник (O): " << state.opponent_agent << "\n";
  ss << "🏆 Победитель: *" << result.winner << "*\n";
  ss << "⏱ Шагов: " << result.steps << "\n\n";

  if (result.winner == "X") {
    ss << "🏆 *ПОБЕДА (X)!*";
  } else if (result.winner == "O") {
    ss << "🤖 *ПОБЕДИЛ ПРОТИВНИК (O)*";
  } else {
    ss << "🤝 *НИЧЬЯ*";
  }

  // Отправка результата с изображением если есть
  if (!result.image_filename.empty()) {
    string local_path = "output/" + result.image_filename;
    if (access(local_path.c_str(), F_OK) == 0) {
      uploadPhoto(chat_id, local_path, ss.str());
    } else if (access(("/app/" + local_path).c_str(), F_OK) == 0) {
      uploadPhoto(chat_id, "/app/" + local_path, ss.str());
    } else if (!result.image_url.empty()) {
      sendPhoto(chat_id, result.image_url, ss.str());
    } else {
      sendMessage(chat_id, ss.str(), Keyboard::createTicTacToeMenu(), true);
    }
  } else if (!result.image_url.empty()) {
    sendPhoto(chat_id, result.image_url, ss.str());
  } else {
    sendMessage(chat_id, ss.str(), Keyboard::createTicTacToeMenu(), true);
  }
}

// ============================================================
//  Игра Мафия
// ============================================================

void TelegramBot::handleMafiaGame(int64_t chat_id, UserState &state) {

  sendMessage(chat_id, "🎭 Запускаю мафию с " + to_string(state.mafia_players) +
                           " игроками...\n"
                           "Это может занять некоторое время.");

  try {
    auto agents = game_manager_.getAvailableMafiaAgents();
    if (agents.empty()) {
      sendMessage(chat_id, "❌ Список агентов пуст. Попробуйте позже.",
                  Keyboard::createMafiaMenu());
      return;
    }

    MafiaGameResult result = game_manager_.runMafiaGame(
        agents, state.mafia_players, true);

    formatMafiaResult(chat_id, result);
  } catch (const std::exception &e) {
    sendMessage(chat_id, "❌ Ошибка при запуске: " + string(e.what()),
                Keyboard::createMafiaMenu());
  } catch (...) {
    sendMessage(chat_id, "❌ Критическая ошибка при запуске игры.",
                Keyboard::createMafiaMenu());
  }
}

void TelegramBot::formatMafiaResult(int64_t chat_id,
                                    const MafiaGameResult &result) {
  stringstream ss;
  ss << "🎭 *Игра в мафию завершена!*\n\n";
  ss << "🏆 Победитель: *";

  if (result.winner == "citizens") {
    ss << "МИРНЫЕ ЖИТЕЛИ* 🎉\n";
  } else if (result.winner == "mafia") {
    ss << "МАФИЯ* 💀\n";
  } else {
    ss << "ОШИБКА*\n";
  }

  ss << "\n📊 Статистика:\n";
  ss << "• Дней: " << result.total_days << "\n";
  ss << "• Выжило: " << result.surviving_players << "\n";
  ss << "• Всего: " << (result.mafia_team.size() + result.citizen_team.size())
     << "\n";

  ss << "\n👥 Мафия:\n";
  for (const auto &m : result.mafia_team)
    ss << "• " << m << "\n";

  ss << "\n👥 Мирные:\n";
  for (const auto &c : result.citizen_team)
    ss << "• " << c << "\n";

  if (!result.killed_players.empty()) {
    ss << "\n⚰️ Убитые:\n";
    for (const auto &k : result.killed_players) {
      if (ss.str().length() + k.length() < 4000)
        ss << "• " << k << "\n";
    }
  }

  sendMessage(chat_id, ss.str(), Keyboard::createMafiaMenu(), true);

  if (!result.chat_log.empty()) {
    sendMafiaChatLog(chat_id, result.chat_log);
  }

  if (!result.image_url.empty()) {
    sendPhoto(chat_id, result.image_url, "🎭 Визуализация");
  }
}

void TelegramBot::sendMafiaChatLog(
    int64_t chat_id, const std::vector<Mafia::ChatMessage> &chat_log) {

  string current_chunk = "💬 Лог чата (Часть 1):\n\n";
  int part = 1;
  int count = 0;

  for (const auto &msg : chat_log) {
    if (msg.is_public) {
      string line = msg.player_name + ": " + msg.text + "\n";

      // Telegram safely supports ~4096 chars. Let's chunk at 3000 bytes.
      if (current_chunk.length() + line.length() > 3000) {
        sendMessage(chat_id, current_chunk, "", false);
        part++;
        current_chunk = "💬 Лог чата (Часть " + to_string(part) + "):\n\n";
      }

      current_chunk += line;
      count++;
    }
  }

  if (current_chunk.length() > 30) { // More than just header
    sendMessage(chat_id, current_chunk, "", false);
  }
}

void TelegramBot::handleBunkerGame(int64_t chat_id, UserState &state) {
  if (state.bet_amount <= 0) {
    sendMessage(chat_id, "⚠️ Сначала сделайте ставку!",
                Keyboard::createBunkerMenu());
    return;
  }

  sendMessage(chat_id, "🛡️ Запускаю Бункер: вместимость " +
                           to_string(state.bunker_capacity) + ", ставка " +
                           to_string(state.bet_amount) +
                           ".\nЭто может занять некоторое время.");

  try {
    auto agents = game_manager_.getAvailableBunkerAgents();
    if (agents.empty()) {
      sendMessage(chat_id, "❌ Список агентов для Бункера пуст.",
                  Keyboard::createBunkerMenu());
      return;
    }

    BunkerGameResult result = game_manager_.runBunkerGame(
        agents, state.bunker_capacity, state.bet_amount);
    formatBunkerResult(chat_id, result, state.bet_amount);
  } catch (const std::exception &e) {
    sendMessage(chat_id, "❌ Ошибка при запуске Бункера: " + string(e.what()),
                Keyboard::createBunkerMenu());
  } catch (...) {
    sendMessage(chat_id, "❌ Критическая ошибка при запуске Бункера.",
                Keyboard::createBunkerMenu());
  }
}

void TelegramBot::formatBunkerResult(int64_t chat_id,
                                     const BunkerGameResult &result,
                                     int bet_amount) {
  stringstream ss;
  ss << "🛡️ *Игра Бункер завершена!*\n\n";
  ss << "🏆 Исход: *";
  if (result.winner == "survive") {
    ss << "Выжившие прошли в бункер* ✅\n";
    ss << "💰 Выигрыш: " << (bet_amount * 2) << " очков\n";
  } else if (result.winner == "disaster") {
    ss << "Катастрофа* 💥\n";
    ss << "Ставка проиграна\n";
  } else {
    ss << "Ошибка*\n";
  }

  ss << "\n📊 Статистика:\n";
  ss << "• Раундов: " << result.total_rounds << "\n";
  ss << "• Выживших: " << result.survivors_count << "\n";
  ss << "• Изгнано: " << result.exiled_players.size() << "\n";

  if (!result.survivors.empty()) {
    ss << "\n🟢 Выжившие:\n";
    for (const auto &p : result.survivors) {
      if (ss.str().length() + p.length() < 3900)
        ss << "• " << p << "\n";
    }
  }

  if (!result.exiled_players.empty()) {
    ss << "\n🔴 Изгнанные:\n";
    for (const auto &p : result.exiled_players) {
      if (ss.str().length() + p.length() < 3900)
        ss << "• " << p << "\n";
    }
  }

  sendMessage(chat_id, ss.str(), Keyboard::createBunkerMenu(), true);

  if (!result.game_log.empty()) {
    string chunk = "📜 Лог Бункера (часть 1):\n\n";
    int part = 1;
    for (const auto &line : result.game_log) {
      string with_newline = line + "\n";
      if (chunk.length() + with_newline.length() > 3000) {
        sendMessage(chat_id, chunk, "", false);
        part++;
        chunk = "📜 Лог Бункера (часть " + to_string(part) + "):\n\n";
      }
      chunk += with_newline;
    }
    if (chunk.length() > 30) {
      sendMessage(chat_id, chunk, "", false);
    }
  }
}

// ============================================================
//  Обработка сообщений
// ============================================================

void TelegramBot::handleMessage(int64_t chat_id, const string &text,
                                const string &username) {

  UserState &state = user_states_[chat_id];

  // ── Главное меню / старт ──────────────────────────────────
  if (text == "/start" || text == "/menu" || text == "◀️ Назад в главное меню") {
    // Сбрасываем состояние при возврате в главное меню
    state = UserState();
    showMainMenu(chat_id);
    return;
  }

  // ── Вход в Крестики-нолики ─────────────────────────────────
  if (text == "🎮 Крестики-нолики") {
    state.game_mode = "tic_tac_toe";
    showTicTacToeMenu(chat_id);
    return;
  }

  // ── Вход в Мафию ───────────────────────────────────────────
  if (text == "🎭 Мафия") {
    state.game_mode = "mafia";
    showMafiaMenu(chat_id);
    return;
  }
  if (text == "🛡️ Бункер") {
    state.game_mode = "bunker";
    showBunkerMenu(chat_id);
    return;
  }

  // ── Заглушки ───────────────────────────────────────────────
  if (text == "📊 Статистика") {
    sendMessage(chat_id, "📊 Статистика — в разработке.",
                Keyboard::createMainMenu());
    return;
  }
  if (text == "⚙️ Настройки") {
    sendMessage(chat_id, "⚙️ Настройки — в разработке.",
                Keyboard::createMainMenu());
    return;
  }

  // =============================================================
  //  КРЕСТИКИ-НОЛИКИ: подменю
  // =============================================================
  if (state.game_mode == "tic_tac_toe") {

    // --- Навигация ---
    if (text == "◀️ Назад") {
      showTicTacToeMenu(chat_id);
      return;
    }

    // --- Выбор агента ---
    if (text == "🤖 Выбрать агента") {
      sendMessage(chat_id,
                  "🤖 Выберите своего агента:", Keyboard::createTTTAgentMenu());
      return;
    }
    if (text == "Random" || text == "Heuristic" || text == "QLearning" ||
        text == "LLM") {
      string agent = text;
      std::transform(agent.begin(), agent.end(), agent.begin(), ::tolower);
      state.selected_agent = agent;
      sendMessage(chat_id, "✅ Агент: " + text,
                  Keyboard::createTicTacToeMenu());
      return;
    }

    // --- Выбор противника ---
    if (text == "🎯 Выбрать противника") {
      sendMessage(chat_id,
                  "🎯 Выберите противника:", Keyboard::createTTTOpponentMenu());
      return;
    }
    if (text == "Противник: Random" || text == "Противник: Heuristic" ||
        text == "Противник: QLearning" || text == "Противник: LLM") {
      string opp = text.substr(text.find(": ") + 2);
      std::transform(opp.begin(), opp.end(), opp.begin(), ::tolower);
      state.opponent_agent = opp;
      sendMessage(chat_id, "✅ Противник: " + text.substr(text.find(": ") + 2),
                  Keyboard::createTicTacToeMenu());
      return;
    }


    // --- Запуск ---
    if (text == "▶️ Запустить игру") {
      handleTicTacToeGame(chat_id, state);
      return;
    }
  }

  // =============================================================
  //  МАФИЯ: подменю
  // =============================================================
  if (state.game_mode == "mafia") {

    // --- Навигация ---
    if (text == "◀️ Назад") {
      showMafiaMenu(chat_id);
      return;
    }

    // --- Количество игроков ---
    if (text == "👥 Количество игроков") {
      sendMessage(chat_id, "👥 Выберите количество игроков:",
                  Keyboard::createMafiaPlayersMenu());
      return;
    }

    if (text.find("игроков") != std::string::npos &&
        text != "👥 Количество игроков") {
      std::string digits;
      for (unsigned char c : text)
        if (std::isdigit(c))
          digits += char(c);

      if (!digits.empty()) {
        int n = std::stoi(digits);
        if (n >= 6 && n <= 12) {
          state.mafia_players = n;
          sendMessage(chat_id, "✅ Игроков: " + to_string(n),
                      Keyboard::createMafiaMenu());
        } else {
          sendMessage(chat_id, "❌ Нужно от 6 до 12 игроков",
                      Keyboard::createMafiaPlayersMenu());
        }
      } else {
        sendMessage(chat_id, "❌ Неверное число игроков",
                    Keyboard::createMafiaPlayersMenu());
      }
      return;
    }


    // --- Правила ---
    if (text == "📋 Правила") {
      string rules = "🎭 *Правила Мафии:*\n\n"
                     "• 6-12 агентов\n"
                     "• Роли: Мафия, Дон, Шериф, Доктор, Мирные\n"
                     "• Ночью мафия убивает, шериф проверяет, доктор лечит\n"
                     "• Днём игроки обсуждают и голосуют\n"
                     "• Мафия побеждает, когда их не меньше мирных\n"
                     "• Мирные побеждают, когда вся мафия изгнана";
      sendMessage(chat_id, rules, Keyboard::createMafiaMenu(), true);
      return;
    }

    // --- Запуск ---
    if (text == "▶️ Запустить мафию") {
      handleMafiaGame(chat_id, state);
      return;
    }
  }

  // =============================================================
  //  БУНКЕР: подменю
  // =============================================================
  if (state.game_mode == "bunker") {
    if (text == "◀️ Назад") {
      showBunkerMenu(chat_id);
      return;
    }

    if (text == "🏚️ Вместимость бункера") {
      sendMessage(chat_id, "🏚️ Выберите вместимость бункера:",
                  Keyboard::createBunkerCapacityMenu());
      return;
    }

    if (text.rfind("Вместимость ", 0) == 0) {
      std::string digits;
      for (unsigned char c : text)
        if (std::isdigit(c))
          digits += char(c);

      if (!digits.empty()) {
        int cap = std::stoi(digits);
        if (cap >= 3 && cap <= 6) {
          state.bunker_capacity = cap;
          sendMessage(chat_id, "✅ Вместимость: " + to_string(cap),
                      Keyboard::createBunkerMenu());
        } else {
          sendMessage(chat_id, "❌ Допустимо от 3 до 6",
                      Keyboard::createBunkerCapacityMenu());
        }
      } else {
        sendMessage(chat_id, "❌ Неверная вместимость",
                    Keyboard::createBunkerCapacityMenu());
      }
      return;
    }

    if (text == "💰 Ставка бункер") {
      sendMessage(chat_id, "💰 Выберите размер ставки:",
                  Keyboard::createBunkerBetsMenu());
      return;
    }

    bool isBunkerBet =
        (text.rfind("Ставка бункер", 0) == 0) ||
        (!text.empty() && std::isdigit((unsigned char)text[0]));

    if (isBunkerBet) {
      std::string digits;
      for (unsigned char c : text)
        if (std::isdigit(c))
          digits += char(c);

      if (!digits.empty()) {
        state.bet_amount = std::stoi(digits);
        sendMessage(chat_id, "✅ Ставка: " + digits + " очков",
                    Keyboard::createBunkerMenu());
      } else {
        sendMessage(chat_id, "❌ Неверная ставка",
                    Keyboard::createBunkerBetsMenu());
      }
      return;
    }

    if (text == "📋 Правила бункера") {
      const string rules =
          "🛡️ *Правила Бункера:*\n\n"
          "• Есть ограниченная вместимость бункера\n"
          "• Каждый раунд: обсуждение и голосование за изгнание\n"
          "• Цель — оставить наиболее полезных для выживания\n"
          "• Когда катастрофа наступает, выживают только поместившиеся\n\n"
          "💰 Выплата x2 при исходе `survive`";
      sendMessage(chat_id, rules, Keyboard::createBunkerMenu(), true);
      return;
    }

    if (text == "▶️ Запустить бункер") {
      handleBunkerGame(chat_id, state);
      return;
    }
  }

  // ── Неизвестная команда ────────────────────────────────────
  sendMessage(chat_id, "🤔 Неизвестная команда. Используйте меню:",
              Keyboard::createMainMenu());
}

// ============================================================
//  Основной цикл
// ============================================================

void TelegramBot::run() {
  cout << "🚀 Запуск бота..." << endl;
  cout << "🔧 Используется токен: " << token_.substr(0, 10) << "..." << endl;

  int64_t last_update_id = 0;

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
      request_params["timeout"] = 35;
      request_params["limit"] = 10;

      Json::StreamWriterBuilder writer;
      writer["indentation"] = "";
      string json_request = Json::writeString(writer, request_params);

      string response = makeRequest("getUpdates", json_request);

      if (response.empty()) {
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        continue;
      }

      Json::Value root;
      Json::CharReaderBuilder reader;
      string errors;
      istringstream response_stream(response);

      if (!Json::parseFromStream(reader, response_stream, &root, &errors)) {
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        continue;
      }

      if (!root["ok"].asBool()) {
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
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