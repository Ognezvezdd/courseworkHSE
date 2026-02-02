#include "config.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <json/json.h>

Config load_config(const std::string &filename) {
  Config config;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Warning: Config file " << filename
              << " not found. Using defaults." << std::endl;
    config.bot_token = "YOUR_BOT_TOKEN_HERE";
    config.api_url = "http://python-api:8000";
  } else {
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errors;

    if (!Json::parseFromStream(reader, file, &root, &errors)) {
      std::cerr << "Error parsing config: " << errors << std::endl;
      config.bot_token = "YOUR_BOT_TOKEN_HERE";
      config.api_url = "http://python-api:8000";
    } else {
      config.bot_token =
          root.get("bot_token", "YOUR_BOT_TOKEN_HERE").asString();
      config.api_url = root.get("api_url", "http://python-api:8000").asString();
    }
  }

  // Приоритет переменной окружения для Docker
  const char *env_api_url = std::getenv("API_URL");
  if (env_api_url) {
    config.api_url = std::string(env_api_url);
  }

  return config;
}