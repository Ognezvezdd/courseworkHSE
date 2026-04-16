#include "game_manager.hpp"
#include "mafia_agent_proxy.hpp"
#include <algorithm>
#include <iostream>
#include <json/json.h>
#include <random>
#include <sstream>

using namespace Mafia;

GameManager::GameManager(const std::string &api_url)
    : api_url_(api_url), http_client_() {
  if (!api_url_.empty() && api_url_.back() == '/') {
    api_url_.pop_back();
  }
}

bool GameManager::checkApiHealth() {
  std::string response = http_client_.get(api_url_ + "/");
  return http_client_.getLastResponseCode() == 200;
}

std::vector<std::string> GameManager::getAvailableAgents() {
  std::string response = http_client_.get(api_url_ + "/agents");

  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "Failed to get agents from API" << std::endl;
    return {"random", "heuristic", "qlearning"};
  }

  std::vector<std::string> agents;
  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    if (root.isMember("agents") && root["agents"].isArray()) {
      for (const auto &agent : root["agents"]) {
        agents.push_back(agent.asString());
      }
    }
  } else {
    std::cerr << "Failed to parse agents JSON: " << errors << std::endl;
  }

  return agents.empty()
             ? std::vector<std::string>{"random", "heuristic", "qlearning"}
             : agents;
}

std::vector<std::string> GameManager::getAvailableMafiaAgents() {
  // Для начала используем те же агенты, что и для крестиков-ноликов
  auto agents = getAvailableAgents();

  // Добавляем специализированных агентов для мафии
  agents.push_back("mafia_random");
  agents.push_back("mafia_conservative");
  agents.push_back("mafia_aggressive");
  agents.push_back("citizen_cautious");
  agents.push_back("citizen_social");

  return agents;
}

std::vector<std::string> GameManager::getAvailableGames() {
  return {"tic_tac_toe_5x5", "mafia"};
}

GameResult GameManager::parseGameResponse(const std::string &json_response) {
  GameResult result;
  result.json_output = json_response;

  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(json_response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    result.winner = root.get("winner", "error").asString();
    result.steps = root.get("steps", 0).asInt();

    std::string relative_url = root.get("image_url", "").asString();
    if (!relative_url.empty()) {
      result.image_url = api_url_ + relative_url;
    }
    result.image_filename = root.get("image_filename", "").asString();
  } else {
    std::cerr << "Failed to parse game response JSON: " << errors << std::endl;
    result.winner = "error";
    result.steps = 0;
  }

  return result;
}

MafiaGameResult
GameManager::parseMafiaResponse(const std::string &json_response) {
  MafiaGameResult result;
  result.json_output = json_response;
  result.json_output = json_response;

  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(json_response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    result.winner = root.get("winner", "error").asString();
    result.total_days = root.get("total_days", 0).asInt();
    result.surviving_players = root.get("surviving_players", 0).asInt();

    // Команды
    if (root.isMember("mafia_team") && root["mafia_team"].isArray()) {
      for (const auto &member : root["mafia_team"]) {
        result.mafia_team.push_back(member.asString());
      }
    }

    if (root.isMember("citizen_team") && root["citizen_team"].isArray()) {
      for (const auto &member : root["citizen_team"]) {
        result.citizen_team.push_back(member.asString());
      }
    }

    // Убитые игроки
    if (root.isMember("killed_players") && root["killed_players"].isArray()) {
      for (const auto &killed : root["killed_players"]) {
        result.killed_players.push_back(killed.asString());
      }
    }

    // Лог игры
    if (root.isMember("game_log") && root["game_log"].isArray()) {
      for (const auto &log_entry : root["game_log"]) {
        result.game_log.push_back(log_entry.asString());
      }
    }

    // Изображение
    std::string relative_url = root.get("image_url", "").asString();
    if (!relative_url.empty()) {
      result.image_url = api_url_ + relative_url;
    }
  } else {
    std::cerr << "Failed to parse mafia response JSON: " << errors << std::endl;
    result.winner = "error";
  }

  return result;
}

GameResult GameManager::runGame(const std::string &agent_x,
                                const std::string &agent_o, int seed) {

  std::stringstream json_body;
  json_body << "{";
  json_body << "\"agent_x\": \"" << agent_x << "\",";
  json_body << "\"agent_o\": \"" << agent_o << "\"";

  if (seed != 0) {
    json_body << ",\"seed\": " << seed;
  }

  json_body << "}";

  std::string response =
      http_client_.post(api_url_ + "/game/play", json_body.str());

  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "API request failed with code: "
              << http_client_.getLastResponseCode() << std::endl;

    GameResult error_result;
    error_result.winner = "error";
    error_result.steps = 0;
    error_result.steps = 0;
    error_result.json_output = "ERROR: API request failed";
    return error_result;
  }

  return parseGameResponse(response);
}

MafiaGameResult
GameManager::runMafiaGame(const std::vector<std::string> &agents,
                          int num_players, bool use_chat) {

  // Выбираем агентов (если их мало - дублируем)
  std::vector<std::string> selected_agent_names;
  std::vector<std::string> pool = agents;

  if (pool.empty()) {
    pool = getAvailableMafiaAgents();
  }

  if (pool.empty()) {
    pool = {"random", "heuristic", "qlearning"};
  }

  while (static_cast<int>(selected_agent_names.size()) < num_players) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pool.begin(), pool.end(), g);

    for (const auto &name : pool) {
      if (static_cast<int>(selected_agent_names.size()) < num_players) {
        selected_agent_names.push_back(name);
      }
    }
  }

  MafiaGameResult result;

  // Создаем и инициализируем игру
  MafiaGame game(num_players);
  auto mafia_agents = createMafiaAgents(selected_agent_names);

  if (!game.initialize(mafia_agents)) {
    result.winner = "error";
    result.json_output = "Failed to initialize game";
    return result;
  }

  // Игровой цикл
  int cycle_count = 0;
  while (!game.isGameOver() && cycle_count < 50) {
    if (!game.executeCycle())
      break;
    cycle_count++;
  }

  if (cycle_count >= 50 && !game.isGameOver()) {
    std::cout << "⚠️ Mafia game reached max cycles (50) and was stopped."
              << std::endl;
  }

  // Результаты
  auto game_data = game.getResult();

  result.winner = game_data.winner;
  result.mafia_team = game_data.mafia_team;
  result.citizen_team = game_data.citizen_team;
  result.killed_players = game_data.killed_players;
  result.chat_log = game_data.chat_log;
  result.game_log = game_data.game_log;
  result.total_days = game_data.total_days;
  result.surviving_players = game_data.surviving_players;

  if (result.winner != "citizens" && result.winner != "mafia")
    result.winner = "error";

  std::cout << "🏁 Mafia Game Finished. Winner: " << result.winner << std::endl;

  return result;
}

bool GameManager::trainAgent(const std::string &agent_type, int episodes,
                             int seed) {

  std::stringstream json_body;
  json_body << "{";
  json_body << "\"agent_type\": \"" << agent_type << "\",";
  json_body << "\"episodes\": " << episodes << ",";
  json_body << "\"seed\": " << seed << ",";
  json_body << "\"opponent_type\": \"random\"";
  json_body << "}";

  std::string response =
      http_client_.post(api_url_ + "/train", json_body.str());

  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "Training request failed" << std::endl;
    return false;
  }

  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    return root.get("success", false).asBool();
  }

  return false;
}

std::vector<std::shared_ptr<Mafia::IMafiaAgent>>
GameManager::createMafiaAgents(const std::vector<std::string> &agent_names) {
  std::vector<std::shared_ptr<Mafia::IMafiaAgent>> agents;

  for (const auto &name : agent_names) {
    agents.push_back(std::make_shared<MafiaAgentProxy>(name, api_url_));
  }

  return agents;
}

std::vector<Mafia::ChatMessage>
GameManager::getMafiaChatHistory(const std::string &game_id) {
  // Заглушка - в реальности это будет запрос к API
  return {};
}
