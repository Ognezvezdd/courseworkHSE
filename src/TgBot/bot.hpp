#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <map>

class TelegramBot {
public:
    TelegramBot(const std::string& token);
    ~TelegramBot();
    
    void run();
    
private:
    std::string token_;
    std::string base_url_;
    
    // Состояния пользователей (user_id -> состояние)
    std::map<int64_t, std::string> user_states_;
    
    // HTTP запросы
    std::string makeRequest(const std::string& method, const std::string& params = "");
    
    // Отправка сообщений
    void sendMessage(int64_t chat_id, const std::string& text, const std::string& reply_markup = "");
    
    // Обработчики
    void handleMessage(int64_t chat_id, const std::string& text, const std::string& username);
    void handleCommand(int64_t chat_id, const std::string& command, const std::string& username);
    
    // Ответы на кнопки
    void showMainMenu(int64_t chat_id, const std::string& username);
    void showAgentsMenu(int64_t chat_id);
    void showGamesMenu(int64_t chat_id);
    void showBetsMenu(int64_t chat_id);
};

#endif