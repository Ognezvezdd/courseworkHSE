#include "bunker_agent_proxy.hpp"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iostream>
#include <json/json.h>
#include <sstream>

namespace Bunker {

static std::string toLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}

static int healthPenalty(Health h) {
  switch (h) {
  case Health::HEALTHY:
    return 0;
  case Health::SICK:
    return 3;
  case Health::INJURED:
    return 4;
  case Health::DISABLED:
    return 6;
  }
  return 0;
}

static int professionValue(Profession p) {
  switch (p) {
  case Profession::DOCTOR:
    return 6;
  case Profession::ENGINEER:
    return 6;
  case Profession::MILITARY:
    return 5;
  case Profession::FIREFIGHTER:
    return 4;
  case Profession::POLICE:
    return 4;
  case Profession::PILOT:
    return 3;
  case Profession::CHEF:
    return 4;
  case Profession::TEACHER:
    return 2;
  case Profession::STUDENT:
    return 2;
  case Profession::HOMELESS:
    return 0;
  case Profession::UNKNOWN:
    return 1;
  }
  return 1;
}

BunkerAgentProxy::BunkerAgentProxy(const std::string &name,
                                   const std::string &api_url)
    : name_(name), api_url_(api_url), my_character_(),
      rng_(static_cast<unsigned>(std::hash<std::string>{}(name) ^
                                static_cast<unsigned>(time(nullptr)))),
      http_client_(std::make_unique<HttpClient>()) {}

void BunkerAgentProxy::setCharacter(const PlayerCharacter &character) {
  my_character_ = character;
}

void BunkerAgentProxy::updateKnowledge(const std::string &info) {
  knowledge_.push_back(info);
}

bool BunkerAgentProxy::shouldUseApi() const {
  const std::string lname = toLower(name_);
  return lname.find("llm") != std::string::npos;
}

std::string BunkerAgentProxy::sendRequestToAPI(const std::string &endpoint,
                                               const std::string &json_data) {
  try {
    std::string response = http_client_->post(api_url_ + endpoint, json_data);
    if (http_client_->getLastResponseCode() != 200) {
      std::cerr << "❌ Bunker API Error: " << http_client_->getLastResponseCode()
                << " for " << endpoint << std::endl;
      return "";
    }
    return response;
  } catch (const std::exception &e) {
    std::cerr << "❌ Bunker API Exception: " << e.what() << std::endl;
    return "";
  }
}

int BunkerAgentProxy::pickVoteTarget(const std::vector<PlayerCharacter> &players,
                                     const PlayerCharacter &me) {
  std::vector<const PlayerCharacter *> candidates;
  candidates.reserve(players.size());
  for (const auto &p : players) {
    if (!p.is_alive)
      continue;
    if (p.player_id == me.player_id)
      continue;
    candidates.push_back(&p);
  }
  if (candidates.empty())
    return -1;

  const std::string lname = toLower(name_);
  if (lname.find("bunker_random") != std::string::npos) {
    return candidates[rng_() % candidates.size()]->player_id;
  }

  // Базовая оценка "полезности" игрока для выживания.
  // Чем выше score, тем полезнее игрок -> меньше шанс изгнать.
  auto score = [&](const PlayerCharacter &p) {
    int s = 0;
    s += professionValue(p.profession) * 4;
    s -= healthPenalty(p.health) * 5;
    s += (p.age <= 55 ? 2 : 0);
    s += static_cast<int>(p.skills.size());
    if (p.secret.has_value())
      s -= 1; // секреты подозрительны
    return s;
  };

  if (lname.find("bunker_aggressive") != std::string::npos) {
    // Агрессивный: целится в самых сильных (угроза).
    const PlayerCharacter *best = candidates.front();
    int bestScore = score(*best);
    for (auto *c : candidates) {
      int sc = score(*c);
      if (sc > bestScore) {
        bestScore = sc;
        best = c;
      }
    }
    return best->player_id;
  }

  if (lname.find("bunker_diplomat") != std::string::npos) {
    // Дипломат: старается не конфликтовать — голосует за "среднего", чтобы
    // минимизировать обиды (избегает самых сильных и самых слабых).
    std::vector<std::pair<int, int>> scored; // (score, id)
    scored.reserve(candidates.size());
    for (auto *c : candidates)
      scored.push_back({score(*c), c->player_id});
    std::sort(scored.begin(), scored.end(),
              [](auto a, auto b) { return a.first < b.first; });
    return scored[scored.size() / 2].second;
  }

  // Heuristic / survivalist по умолчанию: изгоняем наименее полезного.
  const PlayerCharacter *worst = candidates.front();
  int worstScore = score(*worst);
  for (auto *c : candidates) {
    int sc = score(*c);
    if (sc < worstScore) {
      worstScore = sc;
      worst = c;
    }
  }
  return worst->player_id;
}

BunkerAction BunkerAgentProxy::getAction(
    const std::vector<PlayerCharacter> &players,
    const std::vector<ChatMessage> &chat_history,
    GamePhase current_phase, const PlayerCharacter &my_character,
    const std::vector<std::string> &known_info) {

  if (shouldUseApi()) {
    Json::Value request;
    request["agent_name"] = name_;
    request["phase"] = static_cast<int>(current_phase);

    Json::Value players_json(Json::arrayValue);
    for (const auto &p : players) {
      Json::Value j;
      j["player_id"] = p.player_id;
      j["player_name"] = "Player_" + std::to_string(p.player_id);
      j["profession"] = p.getProfessionString();
      j["age"] = p.age;
      j["health"] = p.getHealthString();
      j["survival_score"] = p.survival_score;
      j["utility_score"] = p.utility_score;
      Json::Value skills_json(Json::arrayValue);
      for (const auto &s : p.skills)
        skills_json.append(BunkerUtils::skillToString(s));
      j["skills"] = skills_json;
      j["personality"] = p.getPersonalityString();
      j["is_alive"] = p.is_alive;
      j["is_exiled"] = p.is_exiled;
      players_json.append(j);
    }
    request["players"] = players_json;

    Json::Value chat_json(Json::arrayValue);
    for (const auto &m : chat_history) {
      Json::Value j;
      j["player_id"] = m.player_id;
      j["player_name"] = "Player_" + std::to_string(m.player_id);
      j["text"] = m.text;
      j["timestamp"] = m.timestamp;
      j["is_public"] = m.is_public;
      chat_json.append(j);
    }
    request["chat_history"] = chat_json;

    Json::Value my_json;
    my_json["player_id"] = my_character.player_id;
    my_json["player_name"] = "Player_" + std::to_string(my_character.player_id);
    my_json["profession"] = my_character.getProfessionString();
    my_json["age"] = my_character.age;
    my_json["health"] = my_character.getHealthString();
    my_json["survival_score"] = my_character.survival_score;
    my_json["utility_score"] = my_character.utility_score;
    request["my_character"] = my_json;

    request["agent_type"] = name_;
    request["agent_name"] = "Player_" + std::to_string(my_character.player_id);

    Json::Value known_json(Json::arrayValue);
    for (const auto &i : known_info)
      known_json.append(i);
    request["known_info"] = known_json;
    if (!openai_api_key_.empty()) {
        request["openai_api_key"] = openai_api_key_;
    }

    Json::StreamWriterBuilder writer;
    writer["emitUTF8"] = true;
    writer["indentation"] = "";
    std::string body = Json::writeString(writer, request);
    std::string response = sendRequestToAPI("/bunker/agent_action", body);

    if (!response.empty()) {
      Json::Value root;
      Json::CharReaderBuilder reader;
      std::string errors;
      std::stringstream ss(response);
      if (Json::parseFromStream(reader, ss, &root, &errors)) {
        std::string action = root.get("action_type", "PASS").asString();
        std::transform(action.begin(), action.end(), action.begin(), ::toupper);
        int target_id = root.get("target_id", -1).asInt();
        if (action == "VOTE_EXILE") {
          return BunkerAction(BunkerAction::Type::VOTE_EXILE, target_id, "");
        }
        return BunkerAction(BunkerAction::Type::PASS, -1, "");
      }
    }
  }

  if (current_phase != GamePhase::VOTING) {
    return BunkerAction(BunkerAction::Type::PASS, -1, "");
  }

  int target = pickVoteTarget(players, my_character);
  if (target == -1)
    return BunkerAction(BunkerAction::Type::PASS, -1, "");

  return BunkerAction(BunkerAction::Type::VOTE_EXILE, target, "");
}

std::string BunkerAgentProxy::getChatMessage(
    const std::vector<PlayerCharacter> &players,
    const std::vector<ChatMessage> &chat_history,
    GamePhase current_phase, const PlayerCharacter &my_character,
    const std::vector<std::string> &known_info) {

  if (shouldUseApi()) {
    Json::Value request;
    request["agent_name"] = name_;
    request["phase"] = static_cast<int>(current_phase);

    Json::Value players_json(Json::arrayValue);
    for (const auto &p : players) {
      Json::Value j;
      j["player_id"] = p.player_id;
      j["player_name"] = "Player_" + std::to_string(p.player_id);
      j["profession"] = p.getProfessionString();
      j["age"] = p.age;
      j["health"] = p.getHealthString();
      j["survival_score"] = p.survival_score;
      j["utility_score"] = p.utility_score;
      Json::Value skills_json(Json::arrayValue);
      for (const auto &s : p.skills)
        skills_json.append(BunkerUtils::skillToString(s));
      j["skills"] = skills_json;
      j["personality"] = p.getPersonalityString();
      j["is_alive"] = p.is_alive;
      j["is_exiled"] = p.is_exiled;
      players_json.append(j);
    }
    request["players"] = players_json;

    Json::Value chat_json(Json::arrayValue);
    for (const auto &m : chat_history) {
      Json::Value j;
      j["player_id"] = m.player_id;
      j["player_name"] = "Player_" + std::to_string(m.player_id);
      j["text"] = m.text;
      j["timestamp"] = m.timestamp;
      j["is_public"] = m.is_public;
      chat_json.append(j);
    }
    request["chat_history"] = chat_json;

    Json::Value my_json;
    my_json["player_id"] = my_character.player_id;
    my_json["player_name"] = "Player_" + std::to_string(my_character.player_id);
    my_json["profession"] = my_character.getProfessionString();
    my_json["age"] = my_character.age;
    my_json["health"] = my_character.getHealthString();
    my_json["survival_score"] = my_character.survival_score;
    my_json["utility_score"] = my_character.utility_score;
    request["my_character"] = my_json;
    
    request["agent_type"] = name_;
    request["agent_name"] = "Player_" + std::to_string(my_character.player_id);

    Json::Value known_json(Json::arrayValue);
    for (const auto &i : known_info)
      known_json.append(i);
    request["known_info"] = known_json;
    if (!openai_api_key_.empty()) {
        request["openai_api_key"] = openai_api_key_;
    }

    Json::StreamWriterBuilder writer;
    writer["emitUTF8"] = true;
    writer["indentation"] = "";
    std::string body = Json::writeString(writer, request);
    std::string response = sendRequestToAPI("/bunker/agent_chat", body);

    if (!response.empty()) {
      Json::Value root;
      Json::CharReaderBuilder reader;
      std::string errors;
      std::stringstream ss(response);
      if (Json::parseFromStream(reader, ss, &root, &errors)) {
        return root.get("message", "").asString();
      }
    }
  }

  const std::string lname = toLower(name_);
  if (current_phase != GamePhase::DISCUSSION)
    return "";

  std::ostringstream ss;

  if (lname.find("bunker_survivalist") != std::string::npos ||
      lname.find("bunker_heuristic") != std::string::npos) {
    ss << "Я " << my_character.getProfessionString() << ", "
       << my_character.age << " лет, здоровье: " << my_character.getHealthString()
       << ". Мои навыки: " << my_character.getSkillsString()
       << ". Я полезен(на) для выживания в бункере.";
    return ss.str();
  }

  if (lname.find("bunker_aggressive") != std::string::npos) {
    // Немного провокации: давим на слабых.
    int target = pickVoteTarget(players, my_character);
    const PlayerCharacter *tp = nullptr;
    for (const auto &p : players)
      if (p.player_id == target)
        tp = &p;
    if (tp) {
      ss << "Считаю, что " << tp->player_name
         << " менее полезен(на) для выживания. Предлагаю рассмотреть изгнание.";
      return ss.str();
    }
    return "Давайте трезво оценим, кто реально полезен в бункере.";
  }

  if (lname.find("bunker_diplomat") != std::string::npos) {
    return "Предлагаю говорить по фактам: профессия, здоровье, навыки. Нам нужны "
           "командные игроки, а не конфликт.";
  }

  // random: коротко
  return "Я за рациональный выбор: оставим самых полезных для выживания.";
}

} // namespace Bunker

