#include "game_manager.hpp"
#include <iostream>
#include <sstream>
#include <array>
#include <memory>
#include <cstdio>
#include <algorithm>  // Добавляем для std::remove, std::count
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

GameManager::GameManager(const std::string& python_path, const std::string& script_path)
    : python_path_(python_path), script_path_(script_path) {}

std::string GameManager::executePythonScript(const std::vector<std::string>& args) {
    std::string command = python_path_ + " " + script_path_;
    for (const auto& arg : args) {
        command += " " + arg;
    }
    
    std::cout << "Executing: " << command << std::endl;
    
    std::array<char, 128> buffer;
    std::string result;
    
    // Используем popen для выполнения команды и чтения вывода
    #ifdef _WIN32
        FILE* pipe = _popen(command.c_str(), "r");
    #else
        FILE* pipe = popen(command.c_str(), "r");
    #endif
    
    if (!pipe) {
        return "ERROR: Failed to execute Python script";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    #ifdef _WIN32
        _pclose(pipe);
    #else
        pclose(pipe);
    #endif
    
    return result;
}

GameResult GameManager::runGame(
    const std::string& agent_x,
    const std::string& agent_o,
    int seed) {
    
    GameResult result;
    result.bet_amount = 100;  // По умолчанию
    
    std::vector<std::string> args;
    args.push_back("--agent-x");
    args.push_back(agent_x);
    args.push_back("--agent-o");
    args.push_back(agent_o);
    
    if (seed != 0) {
        args.push_back("--seed");
        args.push_back(std::to_string(seed));
    }
    
    std::string json_output = executePythonScript(args);
    result.json_output = json_output;
    
    // Парсим JSON для получения победителя
    if (json_output.find("ERROR") == 0) {
        result.winner = "error";
        return result;
    }
    
    // Простой парсинг
    size_t winner_pos = json_output.find("\"winner\"");
    if (winner_pos != std::string::npos) {
        size_t start = json_output.find(":", winner_pos) + 1;
        size_t end = json_output.find(",", start);
        if (end == std::string::npos) end = json_output.find("}", start);
        
        if (start != std::string::npos && end != std::string::npos) {
            std::string winner_value = json_output.substr(start, end - start);
            // Убираем кавычки и пробелы
            winner_value.erase(std::remove(winner_value.begin(), winner_value.end(), '\"'), winner_value.end());
            winner_value.erase(std::remove(winner_value.begin(), winner_value.end(), ' '), winner_value.end());
            result.winner = winner_value;
        }
    }
    
    // Подсчет шагов (упрощенный метод)
    size_t step_count = 0;
    for (char c : json_output) {
        if (c == '}') step_count++;
    }
    result.steps = step_count / 2;
    
    // Расчет выигрыша
    if (result.winner == "X") {
        result.win_amount = result.bet_amount * 2;  // Выигрыш x2
    } else if (result.winner == "O") {
        result.win_amount = 0;  // Проигрыш
    } else if (result.winner == "draw") {
        result.win_amount = result.bet_amount;  // Возврат ставки
    } else {
        result.win_amount = 0;  // Ошибка или другое
    }
    
    return result;
}

bool GameManager::trainAgent(
    const std::string& agent_type,
    int episodes,
    int seed) {
    
    std::vector<std::string> args;
    args.push_back("--agent-x");
    args.push_back(agent_type);
    args.push_back("--train");
    args.push_back(std::to_string(episodes));
    
    if (seed != 0) {
        args.push_back("--seed");
        args.push_back(std::to_string(seed));
    }
    
    std::string output = executePythonScript(args);
    return output.find("ERROR") == std::string::npos;
}

std::vector<std::string> GameManager::getAvailableAgents() {
    return {"random", "heuristic", "qlearning"};
}

std::vector<std::string> GameManager::getAvailableGames() {
    return {"tic_tac_toe_5x5"};  // Пока одна игра
}
