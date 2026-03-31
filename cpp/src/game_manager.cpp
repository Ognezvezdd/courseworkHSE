#include "game_manager.hpp"
#include "mafia_agent_proxy.hpp"
#include "bunker_agent_proxy.hpp"
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
  auto agents = getAvailableAgents();
  agents.push_back("mafia_random");
  agents.push_back("mafia_conservative");
  agents.push_back("mafia_aggressive");
  agents.push_back("citizen_cautious");
  agents.push_back("citizen_social");
  return agents;
}

std::vector<std::string> GameManager::getAvailableBunkerAgents() {
  auto agents = getAvailableAgents();
  agents.push_back("bunker_random");
  agents.push_back("bunker_heuristic");
  agents.push_back("bunker_aggressive");
  agents.push_back("bunker_survivalist");
  agents.push_back("bunker_diplomat");
  agents.push_back("bunker_llm");
  return agents;
}

std::vector<std::string> GameManager::getAvailableGames() {
  return {"tic_tac_toe_5x5", "mafia", "bunker"};
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

MafiaGameResult
GameManager::parseMafiaResponse(const std::string &json_response) {
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

    if (root.isMember("killed_players") && root["killed_players"].isArray()) {
      for (const auto &killed : root["killed_players"]) {
        result.killed_players.push_back(killed.asString());
      }
    }

    if (root.isMember("game_log") && root["game_log"].isArray()) {
      for (const auto &log_entry : root["game_log"]) {
        result.game_log.push_back(log_entry.asString());
      }
    }

    std::string relative_url = root.get("image_url", "").asString();
    if (!relative_url.empty()) {
      result.image_url = api_url_ + relative_url;
    }

    if (result.winner == "citizens") {
      result.win_amount = result.bet_amount * 3;
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

BunkerGameResult
GameManager::parseBunkerResponse(const std::string &json_response) {
  BunkerGameResult result;
  result.bet_amount = 100;
  result.win_amount = 0;
  result.json_output = json_response;

  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(json_response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    result.winner = root.get("winner", "error").asString();
    result.total_rounds = root.get("total_rounds", 0).asInt();
    result.survivors_count = root.get("survivors_count", 0).asInt();

    if (root.isMember("survivors") && root["survivors"].isArray()) {
      for (const auto &s : root["survivors"]) {
        result.survivors.push_back(s.asString());
      }
    }

    if (root.isMember("exiled_players") && root["exiled_players"].isArray()) {
      for (const auto &e : root["exiled_players"]) {
        result.exiled_players.push_back(e.asString());
      }
    }

    if (root.isMember("game_log") && root["game_log"].isArray()) {
      for (const auto &log_entry : root["game_log"]) {
        result.game_log.push_back(log_entry.asString());
      }
    }

    if (result.winner == "survive") {
      result.win_amount = result.bet_amount * 2;
    } else if (result.winner == "disaster") {
      result.win_amount = 0;
    } else {
      result.winner = "error";
    }
  } else {
    std::cerr << "Failed to parse bunker response JSON: " << errors << std::endl;
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
    error_result.bet_amount = 100;
    error_result.win_amount = 0;
    error_result.json_output = "ERROR: API request failed";
    return error_result;
  }

  return parseGameResponse(response);
}

MafiaGameResult
GameManager::runMafiaGame(const std::vector<std::string> &agents,
                          int num_players, int bet_amount, bool use_chat) {
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

  // Стараемся включать LLM-агента хотя бы один раз, если он доступен в пуле.
  bool has_llm_in_pool = std::find(pool.begin(), pool.end(), "bunker_llm") != pool.end();
  bool has_llm_selected = std::find(selected_agent_names.begin(), selected_agent_names.end(),
                                    "bunker_llm") != selected_agent_names.end();
  if (has_llm_in_pool && !has_llm_selected && !selected_agent_names.empty()) {
    selected_agent_names[0] = "bunker_llm";
  }

  MafiaGameResult result;
  result.bet_amount = bet_amount;
  result.win_amount = 0;

  MafiaGame game(num_players);
  auto mafia_agents = createMafiaAgents(selected_agent_names);

  if (!game.initialize(mafia_agents)) {
    result.winner = "error";
    result.json_output = "Failed to initialize game";
    return result;
  }

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

  auto game_data = game.getResult();

  result.winner = game_data.winner;
  result.mafia_team = game_data.mafia_team;
  result.citizen_team = game_data.citizen_team;
  result.killed_players = game_data.killed_players;
  result.chat_log = game_data.chat_log;
  result.game_log = game_data.game_log;
  result.total_days = game_data.total_days;
  result.surviving_players = game_data.surviving_players;

  if (result.winner == "citizens")
    result.win_amount = bet_amount * 3;

  std::cout << "🏁 Mafia Game Finished. Winner: " << result.winner << std::endl;

  return result;
}

BunkerGameResult
GameManager::runBunkerGame(const std::vector<std::string> &agents,
                           int bunker_capacity, int bet_amount) {
  std::vector<std::string> selected_agent_names;
  std::vector<std::string> pool = agents;

  if (pool.empty()) {
    pool = getAvailableBunkerAgents();
  }

  if (pool.empty()) {
    pool = {"random", "heuristic", "qlearning"};
  }

  int num_players = std::max(3, bunker_capacity + 1);

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

  BunkerGameResult result;
  result.bet_amount = bet_amount;
  result.win_amount = 0;

  Bunker::BunkerGame game(bunker_capacity);
  auto bunker_agents = createBunkerAgents(selected_agent_names);

  if (!game.initialize(bunker_agents)) {
    result.winner = "error";
    result.json_output = "Failed to initialize Bunker game";
    return result;
  }

  if (!game.startGame()) {
    result.winner = "error";
    result.json_output = "Failed to start Bunker game";
    return result;
  }

  int cycle_count = 0;
  while (!game.isGameOver() && cycle_count < 20) {
    if (!game.executeCycle()) break;
    cycle_count++;
  }

  if (cycle_count >= 20 && !game.isGameOver()) {
    std::cout << "⚠️ Bunker game reached max cycles (20) and was stopped." << std::endl;
  }

  auto game_data = game.getResult();

  result.winner = game_data.winner;
  result.survivors = game_data.survivors;
  result.exiled_players = game_data.exiled_players;
  result.game_log = game_data.game_log;
  result.chat_log = game_data.chat_history;
  result.total_rounds = game_data.total_rounds;
  result.survivors_count = game_data.survivors_count;

  if (result.winner == "survive") {
    result.win_amount = bet_amount * 2;
  }

  std::cout << "🏁 Bunker Game Finished. Winner: " << result.winner 
            << " | Survivors: " << result.survivors_count << std::endl;

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

std::vector<std::shared_ptr<Bunker::IBunkerAgent>>
GameManager::createBunkerAgents(const std::vector<std::string> &agent_names) {
  std::vector<std::shared_ptr<Bunker::IBunkerAgent>> agents;
  for (const auto &name : agent_names) {
    agents.push_back(std::make_shared<Bunker::BunkerAgentProxy>(name, api_url_));
  }
  return agents;
}

std::vector<Mafia::ChatMessage>
GameManager::getMafiaChatHistory(const std::string &game_id) {
  return {};
}