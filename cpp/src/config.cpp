#include "config.hpp"
#include <fstream>
#include <iostream>
#include <json/json.h>

Config load_config(const std::string& filename) {
    Config config;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Config file " << filename << " not found. Using defaults." << std::endl;
        config.bot_token = "YOUR_BOT_TOKEN_HERE";
        config.python_path = "python3";
        config.game_script_path = "../python/run_game.py";
        return config;
    }
    
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;
    
    if (!Json::parseFromStream(reader, file, &root, &errors)) {
        std::cerr << "Error parsing config: " << errors << std::endl;
        config.bot_token = "YOUR_BOT_TOKEN_HERE";
        config.python_path = "python3";
        config.game_script_path = "../python/run_game.py";
        return config;
    }
    
    config.bot_token = root.get("bot_token", "YOUR_BOT_TOKEN_HERE").asString();
    config.python_path = root.get("python_path", "python3").asString();
    config.game_script_path = root.get("game_script_path", "../python/run_game.py").asString();
    
    return config;
}