#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <map>
#include "game_manager.hpp"
#include "user_state.hpp"

class TelegramBot {
public:
    TelegramBot(const std::string& token, GameManager& game_manager);
    ~TelegramBot();
    
    void run();
    
private:
    std::string token_;
    std::string base_url_;
    GameManager& game_manager_;
    
    std::map<int64_t, UserState> user_states_;
    
    // HTTP запросы
    std::string makeRequest(const std::string& method, const std::string& params = "");
    
    // Отправка сообщений
    void sendMessage(int64_t chat_id, const std::string& text, 
                    const std::string& reply_markup = "", bool markdown = true);
    
    // Обработчики
    void handleMessage(int64_t chat_id, const std::string& text, const std::string& username);
    void handleCommand(int64_t chat_id, const std::string& command, const std::string& username);
    
    // Меню
    void showMainMenu(int64_t chat_id, const std::string& username);
    void showAgentsMenu(int64_t chat_id);
    void showGamesMenu(int64_t chat_id);
    void showBetsMenu(int64_t chat_id);
    void showPlayMenu(int64_t chat_id);
    
    // Обработка выбора
    void handleAgentSelection(int64_t chat_id, const std::string& agent);
    void handleGameSelection(int64_t chat_id, const std::string& game);
    void handleBetSelection(int64_t chat_id, const std::string& bet_text);
    
    // Игровая логика
    void startGame(int64_t chat_id);
};

#endif