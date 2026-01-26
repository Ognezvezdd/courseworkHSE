#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

#include <string>
#include <vector>
#include <map>

struct GameResult {
    std::string winner;  // "X", "O", "draw", "error"
    int steps;
    int bet_amount;
    int win_amount;
    std::string json_output;  // Результат игры в JSON
};

class GameManager {
public:
    GameManager(const std::string& python_path, const std::string& script_path);
    
    // Запуск игры между двумя агентами
    GameResult runGame(
        const std::string& agent_x,
        const std::string& agent_o,
        int seed = 0
    );
    
    // Обучение агента
    bool trainAgent(
        const std::string& agent_type,
        int episodes = 1000,
        int seed = 42
    );
    
    // Доступные агенты
    static std::vector<std::string> getAvailableAgents();
    
    // Доступные игры (на будущее)
    static std::vector<std::string> getAvailableGames();
    
private:
    std::string python_path_;
    std::string script_path_;
    
    std::string executePythonScript(const std::vector<std::string>& args);
};

#endif
