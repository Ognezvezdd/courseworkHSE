#include "mafia_agent_proxy.hpp"
#include <algorithm>
#include <iostream>
#include <json/json.h>
#include <random>
#include <sstream>

using namespace Mafia;

MafiaAgentProxy::MafiaAgentProxy(const std::string &name,
                                 const std::string &api_url)
    : name_(name), api_url_(api_url), role_(Role::CITIZEN) {
  // Инициализация
}

PlayerAction
MafiaAgentProxy::getAction(const std::vector<Player> &players,
                           const std::vector<ChatMessage> &chat_history,
                           Phase current_phase,
                           const std::vector<std::string> &known_info) {

  // Формируем JSON запрос к Python API
  Json::Value request;
  request["agent_name"] = name_;
  request["phase"] = static_cast<int>(current_phase);
  request["role"] = RoleUtils::roleToString(role_);

  // Информация об игроках
  Json::Value players_json(Json::arrayValue);
  for (const auto &player : players) {
    Json::Value p;
    p["id"] = player.id;
    p["name"] = player.name;
    p["role"] = RoleUtils::roleToString(player.role);
    p["is_alive"] = player.is_alive;
    p["is_protected"] = player.is_protected;
    p["votes_against"] = player.votes_against;
    players_json.append(p);
  }
  request["players"] = players_json;

  // История чата
  Json::Value chat_json(Json::arrayValue);
  for (const auto &msg : chat_history) {
    Json::Value m;
    m["player_id"] = msg.player_id;
    m["player_name"] = msg.player_name;
    m["player_role"] = msg.player_role;
    m["text"] = msg.text;
    m["timestamp"] = msg.timestamp;
    m["is_night"] = msg.is_night;
    m["is_public"] = msg.is_public;
    chat_json.append(m);
  }
  request["chat_history"] = chat_json;

  // Известная информация
  Json::Value known_info_json(Json::arrayValue);
  for (const auto &info : known_info) {
    known_info_json.append(info);
  }
  request["known_info"] = known_info_json;

  // Отправляем запрос
  Json::StreamWriterBuilder writer;
  std::string json_request = Json::writeString(writer, request);

  std::string response = sendRequestToAPI("/mafia/agent_action", json_request);

  // Парсим ответ
  if (response.empty()) {
    // Возвращаем случайное действие в случае ошибки
    std::vector<int> alive_players;
    for (const auto &player : players) {
      if (player.is_alive && player.id != 0) { // 0 - системный
        alive_players.push_back(player.id);
      }
    }

    if (alive_players.empty()) {
      return PlayerAction(PlayerAction::Type::PASS);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, alive_players.size() - 1);

    PlayerAction::Type action_type = PlayerAction::Type::PASS;

    // Определяем тип действия в зависимости от фазы и роли
    switch (current_phase) {
    case Phase::NIGHT_MAFIA:
      if (RoleUtils::isMafiaRole(role_)) {
        action_type = PlayerAction::Type::MAFIA_KILL;
      }
      break;
    case Phase::NIGHT_SHERIFF:
      if (role_ == Role::SHERIFF) {
        action_type = PlayerAction::Type::SHERIFF_CHECK;
      }
      break;
    case Phase::NIGHT_DOCTOR:
      if (role_ == Role::DOCTOR) {
        action_type = PlayerAction::Type::DOCTOR_HEAL;
      }
      break;
    case Phase::DAY_VOTING:
      action_type = PlayerAction::Type::VOTE_KILL;
      break;
    default:
      action_type = PlayerAction::Type::PASS;
    }

    return PlayerAction(action_type, alive_players[dis(gen)]);
  }

  // Парсим JSON ответ
  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    std::string action_str = root.get("action_type", "pass").asString();
    int target_id = root.get("target_id", -1).asInt();

    PlayerAction::Type action_type = PlayerAction::Type::PASS;

    if (action_str == "vote_kill")
      action_type = PlayerAction::Type::VOTE_KILL;
    else if (action_str == "mafia_kill")
      action_type = PlayerAction::Type::MAFIA_KILL;
    else if (action_str == "sheriff_check")
      action_type = PlayerAction::Type::SHERIFF_CHECK;
    else if (action_str == "doctor_heal")
      action_type = PlayerAction::Type::DOCTOR_HEAL;
    else if (action_str == "don_check")
      action_type = PlayerAction::Type::DON_CHECK;

    return PlayerAction(action_type, target_id);
  }

  return PlayerAction(PlayerAction::Type::PASS);
}

std::string MafiaAgentProxy::getChatMessage(
    const std::vector<Player> & /*players*/,
    const std::vector<ChatMessage> & /*chat_history*/, Phase current_phase) {

  // Формируем JSON запрос для получения сообщения
  Json::Value request;
  request["agent_name"] = name_;
  request["phase"] = static_cast<int>(current_phase);
  request["role"] = RoleUtils::roleToString(role_);

  // Отправляем запрос
  Json::StreamWriterBuilder writer;
  std::string json_request = Json::writeString(writer, request);

  std::string response = sendRequestToAPI("/mafia/agent_chat", json_request);

  if (response.empty()) {
    // Заглушка - случайные фразы в зависимости от роли и фазы
    static const std::vector<std::string> citizen_phrases = {
        "Я думаю, мы должны обсудить последние события...",
        "Кто-то ведет себя подозрительно.",
        "Давайте проголосуем за самого подозрительного.",
        "Я мирный житель, честно!", "Ночь была страшной..."};

    static const std::vector<std::string> mafia_phrases = {
        "Я тоже мирный, как и все.", "Кто-то явно врет...",
        "Давайте послушаем аргументы.", "Я голосую за него!",
        "Нужно быть осторожнее..."};

    std::random_device rd;
    std::mt19937 gen(rd());

    if (RoleUtils::isMafiaRole(role_)) {
      std::uniform_int_distribution<> dis(0, (int)mafia_phrases.size() - 1);
      return mafia_phrases[dis(gen)];
    } else {
      std::uniform_int_distribution<> dis(0, (int)citizen_phrases.size() - 1);
      return citizen_phrases[dis(gen)];
    }
  }

  // Парсим ответ
  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(response);

  if (Json::parseFromStream(reader, ss, &root, &errors)) {
    return root.get("message", "").asString();
  }

  return "";
}

void MafiaAgentProxy::updateKnowledge(const std::string &info) {
  knowledge_.push_back(info);
}

std::string MafiaAgentProxy::sendRequestToAPI(const std::string &endpoint,
                                              const std::string &json_data) {
  // Заглушка - в реальности это будет HTTP запрос к Python API
  std::cout << "MafiaAgentProxy: Would send to " << api_url_ + endpoint
            << " data: " << json_data.substr(0, 100) << "..." << std::endl;

  // В реальной реализации здесь будет использование HttpClient
  return "";
}