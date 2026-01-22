#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
    std::string bot_token;
    std::string python_path = "python";
    std::string game_script_path = ""; // путь к скрипту
};

Config load_config(const std::string& filename = "config.json");

#endif