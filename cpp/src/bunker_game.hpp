#pragma once

#include "bunker_models.hpp"
#include "bunker_agent_interface.hpp"
#include <vector>
#include <memory>
#include <random>
#include <map>

namespace Bunker {

class BunkerGame {
public:
    BunkerGame(int bunker_capacity = 4);
    ~BunkerGame() = default;
    
    // Инициализация игры
    bool initialize(const std::vector<std::shared_ptr<IBunkerAgent>>& agents);
    
    // Добавить игрока (через агента)
    bool addPlayer(std::shared_ptr<IBunkerAgent> agent);
    
    // Запустить игру
    bool startGame();
    
    // Выполнить полный цикл (обсуждение + голосование)
    bool executeCycle();
    
    // Получить результат
    BunkerResult getResult() const;
    
    // Проверить, окончена ли игра
    bool isGameOver() const { return game_over_; }
    
    // Получить текущую фазу
    GamePhase getCurrentPhase() const { return current_phase_; }
    
    // Получить текущий раунд
    int getCurrentRound() const { return current_round_; }
    
    // Получить список живых игроков
    std::vector<int> getAlivePlayerIds() const;
    
    // Получить персонажа по ID
    const PlayerCharacter* getPlayerById(int player_id) const;
    
    // Добавить сообщение в чат
    void addChatMessage(int player_id, const std::string& message, bool is_public);
    
    // Голосование (вызывается из executeCycle)
    void submitVote(int voter_id, int target_id);
    
private:
    // Генерация случайных характеристик
    PlayerCharacter generateRandomCharacter(int player_id, const std::string& player_name);
    
    // Найти индекс игрока по ID
    int findPlayerIndex(int player_id) const;
    
    // Изгнать игрока
    void exilePlayer(int player_id);
    
    // Проверить условия победы
    bool checkWinCondition();
    
    // Выполнить фазу обсуждения
    void executeDiscussionPhase();
    
    // Выполнить фазу голосования
    void executeVotingPhase();
    
    // Разрешить голосование
    int resolveVoting();
    
    // Отправить системное сообщение
    void broadcastMessage(const std::string& message);
    
    // Отправить личное сообщение
    void sendPrivateMessage(int player_id, const std::string& message);
    
    // Получить текущее время
    std::string getCurrentTime() const;
    
    // Добавить в лог
    void addToLog(const std::string& message);
    
private:
    std::vector<PlayerCharacter> players_;
    std::vector<std::shared_ptr<IBunkerAgent>> agents_;
    std::vector<ChatMessage> chat_history_;
    std::vector<std::string> game_log_;
    
    std::map<int, int> voting_results_;  // target_id -> votes
    
    GamePhase current_phase_;
    int current_round_;
    int bunker_capacity_;
    int disaster_timer_;  // Раундов до катастрофы
    bool game_over_;
    std::string winner_;
    
    std::mt19937 rng_;
};

} // namespace Bunker