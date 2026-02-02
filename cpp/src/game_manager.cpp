#include "game_manager.hpp"
#include <algorithm>
#include <iostream>
#include <json/json.h>
#include <sstream>

GameManager::GameManager(const std::string &api_url)
    : api_url_(api_url), http_client_() {
  // Убираем trailing slash если есть
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
    return {"random", "heuristic", "qlearning"}; // Fallback
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

std::vector<std::string> GameManager::getAvailableGames() {
  return {"tic_tac_toe_5x5"};
}

GameResult GameManager::parseGameResponse(const std::string &json_response) {
  GameResult result;
  result.bet_amount = 100; // По умолчанию
  result.json_output = json_response;

  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(json_response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    result.winner = root.get("winner", "error").asString();
    result.steps = root.get("steps", 0).asInt();
  } else {
    std::cerr << "Failed to parse game response JSON: " << errors << std::endl;
    result.winner = "error";
    result.steps = 0;
  }

  // Расчет выигрыша
  if (result.winner == "X") {
    result.win_amount = result.bet_amount * 2; // Выигрыш x2
  } else if (result.winner == "O") {
    result.win_amount = 0; // Проигрыш
  } else if (result.winner == "draw") {
    result.win_amount = result.bet_amount; // Возврат ставки
  } else {
    result.win_amount = 0; // Ошибка
    result.winner = "error";
  }

  return result;
}

GameResult GameManager::runGame(const std::string &agent_x,
                                const std::string &agent_o, int seed) {

  // Формируем JSON запрос
  std::stringstream json_body;
  json_body << "{";
  json_body << "\"agent_x\": \"" << agent_x << "\",";
  json_body << "\"agent_o\": \"" << agent_o << "\"";

  if (seed != 0) {
    json_body << ",\"seed\": " << seed;
  }

  json_body << "}";

  std::cout << "Sending POST to " << api_url_ << "/game/play" << std::endl;
  std::cout << "Body: " << json_body.str() << std::endl;

  // Отправляем POST запрос
  std::string response =
      http_client_.post(api_url_ + "/game/play", json_body.str());

  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "API request failed with code: "
              << http_client_.getLastResponseCode() << std::endl;
    std::cerr << "Response: " << response << std::endl;

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

bool GameManager::trainAgent(const std::string &agent_type, int episodes,
                             int seed) {

  // Формируем JSON запрос
  std::stringstream json_body;
  json_body << "{";
  json_body << "\"agent_type\": \"" << agent_type << "\",";
  json_body << "\"episodes\": " << episodes << ",";
  json_body << "\"seed\": " << seed << ",";
  json_body << "\"opponent_type\": \"random\"";
  json_body << "}";

  std::cout << "Training agent via API..." << std::endl;

  // Отправляем POST запрос
  std::string response =
      http_client_.post(api_url_ + "/train", json_body.str());

  if (http_client_.getLastResponseCode() != 200) {
    std::cerr << "Training request failed" << std::endl;
    return false;
  }

  // Проверяем успешность обучения
  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    return root.get("success", false).asBool();
  }

  return false;
}
