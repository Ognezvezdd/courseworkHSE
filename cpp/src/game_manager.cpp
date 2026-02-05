#include "game_manager.hpp"
#include "mafia_agent_proxy.hpp"
#include <algorithm>
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <random>

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

  return agents.empty() ? 
         std::vector<std::string>{"random", "heuristic", "qlearning"} : agents;
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
  result.bet_amount = 100;
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

  if (result.winner == "X") {
    result.win_amount = result.bet_amount * 2;
  } else if (result.winner == "O") {
    result.win_amount = 0;
  } else if (result.winner == "draw") {
    result.win_amount = result.bet_amount;
  } else {
    result.win_amount = 0;
    result.winner = "error";
  }

  return result;
}

MafiaGameResult GameManager::parseMafiaResponse(const std::string &json_response) {
  MafiaGameResult result;
  result.json_output = json_response;
  result.bet_amount = 100;
  result.win_amount = 0;

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
    
    // Расчет выигрыша
    if (result.winner == "citizens") {
      result.win_amount = result.bet_amount * 3; // Больший выигрыш за сложную игру
    } else if (result.winner == "mafia") {
      result.win_amount = 0;
    } else {
      result.winner = "error";
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

  std::cout << "Sending POST to " << api_url_ << "/game/play" << std::endl;

  std::string response =
      http_client_.post(api_url_ + "/game/play", json_body.str());

  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "API request failed with code: "
              << http_client_.getLastResponseCode() << std::endl;

    GameResult error_result;
    error_result.winner = "error";
    error_result.steps = 0;
    error_result.bet_amount = 100;
    error_result.win_amount = 0;
    error_result.json_output = "ERROR: API request failed";
    return error_result;
  }

  return parseGameResponse(response);
}

MafiaGameResult GameManager::runMafiaGame(const std::vector<std::string>& agents, 
                                         int num_players,
                                         int bet_amount,
                                         bool use_chat) {
  
  if (agents.size() < static_cast<size_t>(num_players)) {
    std::cerr << "Not enough agents provided for mafia game" << std::endl;
    
    MafiaGameResult error_result;
    error_result.winner = "error";
    error_result.bet_amount = bet_amount;
    error_result.win_amount = 0;
    error_result.json_output = "ERROR: Not enough agents";
    return error_result;
  }
  
  // Выбираем случайных агентов из списка
  std::vector<std::string> selected_agents;
  std::sample(agents.begin(), agents.end(), std::back_inserter(selected_agents),
              num_players, std::mt19937{std::random_device{}()});
  
  // Формируем JSON запрос для API
  std::stringstream json_body;
  json_body << "{";
  json_body << "\"game_type\": \"mafia\",";
  json_body << "\"num_players\": " << num_players << ",";
  json_body << "\"use_chat\": " << (use_chat ? "true" : "false") << ",";
  json_body << "\"agents\": [";
  
  for (size_t i = 0; i < selected_agents.size(); ++i) {
    json_body << "\"" << selected_agents[i] << "\"";
    if (i < selected_agents.size() - 1) {
      json_body << ",";
    }
  }
  json_body << "]";
  json_body << "}";
  
  std::cout << "Sending POST to " << api_url_ << "/mafia/play" << std::endl;
  std::cout << "Body: " << json_body.str() << std::endl;
  
  // Отправляем запрос к Python API
  std::string response = http_client_.post(api_url_ + "/mafia/play", json_body.str());
  
  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "Mafia API request failed with code: "
              << http_client_.getLastResponseCode() << std::endl;
    
    MafiaGameResult error_result;
    error_result.winner = "error";
    error_result.bet_amount = bet_amount;
    error_result.win_amount = 0;
    error_result.json_output = "ERROR: Mafia API request failed";
    
    // Локальная заглушка для тестирования
    std::cout << "Using local mafia simulation..." << std::endl;
    
    // Создаем локальную игру для тестирования
    MafiaGame game(num_players);
    auto mafia_agents = createMafiaAgents(selected_agents);
    
    if (mafia_agents.size() >= static_cast<size_t>(num_players)) {
      game.initialize(mafia_agents);
      
      while (!game.isGameOver()) {
        if (!game.executeCycle()) {
          break;
        }
      }
      
      auto result_data = game.getResult();
      
      error_result.winner = result_data.winner;
      error_result.mafia_team = result_data.mafia_team;
      error_result.citizen_team = result_data.citizen_team;
      error_result.killed_players = result_data.killed_players;
      error_result.chat_log = result_data.chat_log;
      error_result.game_log = result_data.game_log;
      error_result.total_days = result_data.total_days;
      error_result.surviving_players = result_data.surviving_players;
      
      if (error_result.winner == "citizens") {
        error_result.win_amount = bet_amount * 3;
      }
    }
    
    return error_result;
  }
  
  auto result = parseMafiaResponse(response);
  result.bet_amount = bet_amount;
  
  if (result.winner == "citizens") {
    result.win_amount = bet_amount * 3;
  }
  
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

  std::cout << "Training agent via API..." << std::endl;

  std::string response = http_client_.post(api_url_ + "/train", json_body.str());

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
GameManager::createMafiaAgents(const std::vector<std::string>& agent_names) {
  std::vector<std::shared_ptr<Mafia::IMafiaAgent>> agents;
  
  for (const auto& name : agent_names) {
    agents.push_back(std::make_shared<MafiaAgentProxy>(name, api_url_));
  }
  
  return agents;
}

std::vector<Mafia::ChatMessage> GameManager::getMafiaChatHistory(const std::string& game_id) {
  // Заглушка - в реальности это будет запрос к API
  return {};
}
