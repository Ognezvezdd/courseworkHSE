#ifndef BOT_HPP
#define BOT_HPP

#include "game_manager.hpp"
#include "user_state.hpp"
#include <map>
#include <string>

class TelegramBot {
public:
  TelegramBot(const std::string &token, GameManager &game_manager);
  ~TelegramBot();

  void run();

private:
  // HTTP / Telegram API
  std::string makeRequest(const std::string &method,
                          const std::string &params = "");
  void sendMessage(int64_t chat_id, const std::string &text,
                   const std::string &reply_markup = "", bool markdown = false);
  void sendPhoto(int64_t chat_id, const std::string &photo_url,
                 const std::string &caption = "");
  void uploadPhoto(int64_t chat_id, const std::string &file_path,
                   const std::string &caption = "");

  // Обработка сообщений
  void handleMessage(int64_t chat_id, const std::string &text,
                     const std::string &username);

  // Показ меню
  void showMainMenu(int64_t chat_id);
  void showTicTacToeMenu(int64_t chat_id);
  void showMafiaMenu(int64_t chat_id);

  // Обработчики игр
  void handleTicTacToeGame(int64_t chat_id, UserState &state);
  void handleMafiaGame(int64_t chat_id, UserState &state);

  // Вспомогательные
  void formatMafiaResult(int64_t chat_id, const MafiaGameResult &result);
  void sendMafiaChatLog(int64_t chat_id,
                        const std::vector<Mafia::ChatMessage> &chat_log);

private:
  std::string token_;
  std::string base_url_;
  GameManager &game_manager_;
  std::map<int64_t, UserState> user_states_;
};

#endif // BOT_HPP