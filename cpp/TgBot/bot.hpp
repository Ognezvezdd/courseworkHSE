#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include "game_manager.hpp"

class TelegramBot {
public:
    TelegramBot(const std::string& token, GameManager& game_manager);
    ~TelegramBot();
    
    void run();
    
private:
    std::string makeRequest(const std::string& method, const std::string& params = "");
    void sendMessage(int64_t chat_id, const std::string& text, 
                     const std::string& reply_markup = "", bool markdown = false);
    
    void handleMessage(int64_t chat_id, const std::string& text, const std::string& username);
    
    void showMainMenu(int64_t chat_id);
    void showAgentsMenu(int64_t chat_id);
    void showGamesMenu(int64_t chat_id);
    void showBetsMenu(int64_t chat_id);
    void showPlayMenu(int64_t chat_id);
    
private:
    std::string token_;
    std::string base_url_;
    GameManager& game_manager_;
};

#endif 