#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
  std::string bot_token;
  std::string api_url = "http://python-api:8000"; // URL Python API
};

Config load_config(const std::string &filename = "config.json");

#endif