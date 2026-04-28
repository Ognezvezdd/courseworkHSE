#include "mafia_agent_proxy.hpp"
#include <algorithm>
#include <iostream>
#include <json/json.h>
#include <map>
#include <random>
#include <sstream>

using namespace Mafia;

MafiaAgentProxy::MafiaAgentProxy(const std::string &name,
                                 const std::string &api_url)
    : name_(name), api_url_(api_url), role_(Role::CITIZEN) {
  http_client_ = std::make_unique<HttpClient>();
}

// ─────────────────────────────────────────────────────────────────
// Вспомогательный метод: строим анонимный маппинг id → Player_N
// ─────────────────────────────────────────────────────────────────
static std::map<int, std::string>
buildAliasMap(const std::vector<Player> &players) {
  // Сортируем по id для стабильного порядка
  std::vector<const Player *> sorted;
  for (const auto &p : players)
    sorted.push_back(&p);
  std::sort(sorted.begin(), sorted.end(),
            [](const Player *a, const Player *b) { return a->id < b->id; });

  std::map<int, std::string> alias_map;
  for (size_t i = 0; i < sorted.size(); ++i) {
    alias_map[sorted[i]->id] = "Player_" + std::to_string(i + 1);
  }
  return alias_map;
}

// ─────────────────────────────────────────────────────────────────
// Строим JSON запрос к Python API
// ─────────────────────────────────────────────────────────────────
Json::Value MafiaAgentProxy::buildRequest(
    const std::vector<Player> &players,
    const std::vector<ChatMessage> &chat_history,
    Phase current_phase) const {

  auto alias_map = buildAliasMap(players);

  Json::Value request;
  request["agent_name"] = name_;
  request["phase"] = static_cast<int>(current_phase);
  request["role"] = RoleUtils::roleToString(role_);
  request["my_id"] = my_player_id_;

  // ── Игроки: анонимизированные имена, роль NOT leaked в публичном поле ──
  Json::Value players_json(Json::arrayValue);
  for (const auto &player : players) {
    Json::Value p;
    p["id"] = player.id;
    p["name"] = alias_map.count(player.id) ? alias_map.at(player.id) : player.name;
    // Роль передаём только если это ТЕКУЩИЙ агент (для логики) или мафия-союзники
    bool is_me = (player.id == my_player_id_);
    bool is_ally = false;
    for (int mid : mafia_team_ids_) {
      if (mid == player.id) { is_ally = true; break; }
    }
    if (is_me || is_ally) {
      p["role"] = RoleUtils::roleToString(player.role);
    } else {
      p["role"] = "unknown"; // Не раскрываем роли чужих игроков
    }
    p["is_alive"] = player.is_alive;
    p["is_protected"] = player.is_protected;
    p["votes_against"] = player.votes_against;
    players_json.append(p);
  }
  request["players"] = players_json;

  // ── История чата: только публичные, без системных, имена анонимизированы ──
  Json::Value chat_json(Json::arrayValue);
  for (const auto &msg : chat_history) {
    if (msg.player_id == -1) continue; // Пропускаем системные
    if (!msg.is_public) continue;      // Пропускаем приватные

    Json::Value m;
    m["player_id"] = msg.player_id;
    std::string alias = alias_map.count(msg.player_id)
                            ? alias_map.at(msg.player_id)
                            : msg.player_name;
    m["player_name"] = alias;
    m["player_role"] = ""; // НЕ раскрываем роль через чат
    m["text"] = msg.text;
    m["timestamp"] = msg.timestamp;
    m["is_night"] = msg.is_night;
    m["is_public"] = msg.is_public;
    chat_json.append(m);
  }
  request["chat_history"] = chat_json;

  // ── Личная информация агента ──
  Json::Value known_info_json(Json::arrayValue);
  for (const auto &info : knowledge_) {
    known_info_json.append(info);
  }
  request["known_info"] = known_info_json;

  // ── История голосований ──
  Json::Value voting_json(Json::arrayValue);
  for (const auto &record : voting_history_) {
    Json::Value r;
    r["day"] = record.day;
    Json::Value votes_obj(Json::objectValue);
    for (const auto &[pname, cnt] : record.votes) {
      votes_obj[pname] = cnt;
    }
    r["votes"] = votes_obj;
    r["exiled"] = record.exiled;
    voting_json.append(r);
  }
  request["voting_history"] = voting_json;

  // ── Выбывшие игроки (анонимизированные) ──
  Json::Value elim_json(Json::arrayValue);
  for (const auto &e : eliminated_players_) {
    elim_json.append(e);
  }
  request["eliminated_players"] = elim_json;

  // ── Союзники мафии (IDs) ──
  Json::Value mafia_json(Json::arrayValue);
  for (int mid : mafia_team_ids_) {
    mafia_json.append(mid);
  }
  request["mafia_team_ids"] = mafia_json;

  // ── Раскрытые роли (IDs) ──
  Json::Value known_roles_json(Json::objectValue);
  for (const auto &[pid, role] : known_roles_) {
    known_roles_json[std::to_string(pid)] = role;
  }
  request["known_roles"] = known_roles_json;

  return request;
}

// ─────────────────────────────────────────────────────────────────
// Парсим ответ PlayerAction из JSON строки
// ─────────────────────────────────────────────────────────────────
static PlayerAction parseActionFromJson(const std::string &response,
                                        const std::vector<Player> &players,
                                        Phase current_phase, Role role) {
  if (response.empty()) {
    return PlayerAction(PlayerAction::Type::PASS);
  }

  Json::Value root;
  Json::CharReaderBuilder reader;
  std::string errors;
  std::stringstream ss(response);

  if (!Json::parseFromStream(reader, ss, &root, &errors)) {
    std::cerr << "❌ JSON parse error: " << errors << std::endl;
    return PlayerAction(PlayerAction::Type::PASS);
  }

  std::string action_str = root.get("action_type", "pass").asString();
  std::transform(action_str.begin(), action_str.end(), action_str.begin(),
                 ::tolower);
  int target_id = root.get("target_id", -1).asInt();
  std::string text_msg = root.get("text_message", "").asString();

  PlayerAction::Type action_type = PlayerAction::Type::PASS;
  if (action_str == "vote_kill")      action_type = PlayerAction::Type::VOTE_KILL;
  else if (action_str == "mafia_kill")  action_type = PlayerAction::Type::MAFIA_KILL;
  else if (action_str == "sheriff_check") action_type = PlayerAction::Type::SHERIFF_CHECK;
  else if (action_str == "doctor_heal")   action_type = PlayerAction::Type::DOCTOR_HEAL;
  else if (action_str == "don_check")     action_type = PlayerAction::Type::DON_CHECK;
  else if (action_str == "chat_message")  action_type = PlayerAction::Type::CHAT_MESSAGE;

  // Валидация target_id — должен быть живым игроком
  if (target_id != -1) {
    bool valid = false;
    for (const auto &p : players) {
      if (p.id == target_id && p.is_alive) {
        valid = true;
        break;
      }
    }
    if (!valid) {
      // Выбираем случайного живого (не себя)
      std::vector<int> alive;
      for (const auto &p : players)
        if (p.is_alive)
          alive.push_back(p.id);
      if (!alive.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        target_id = alive[std::uniform_int_distribution<>(0, alive.size() - 1)(gen)];
      } else {
        target_id = -1;
      }
    }
  }

  return PlayerAction(action_type, target_id, text_msg);
}

// ─────────────────────────────────────────────────────────────────
// Фоллбэк: случайное действие
// ─────────────────────────────────────────────────────────────────
static PlayerAction fallbackAction(const std::vector<Player> &players,
                                   Phase current_phase, Role role) {
  std::vector<int> alive;
  for (const auto &p : players)
    if (p.is_alive)
      alive.push_back(p.id);

  if (alive.empty())
    return PlayerAction(PlayerAction::Type::PASS);

  std::random_device rd;
  std::mt19937 gen(rd());
  int target = alive[std::uniform_int_distribution<>(0, alive.size() - 1)(gen)];

  switch (current_phase) {
  case Phase::NIGHT_MAFIA:
    if (RoleUtils::isMafiaRole(role))
      return PlayerAction(PlayerAction::Type::MAFIA_KILL, target);
    break;
  case Phase::NIGHT_SHERIFF:
    if (role == Role::SHERIFF)
      return PlayerAction(PlayerAction::Type::SHERIFF_CHECK, target);
    break;
  case Phase::NIGHT_DOCTOR:
    if (role == Role::DOCTOR)
      return PlayerAction(PlayerAction::Type::DOCTOR_HEAL, target);
    break;
  case Phase::DAY_VOTING:
    return PlayerAction(PlayerAction::Type::VOTE_KILL, target);
  default:
    break;
  }
  return PlayerAction(PlayerAction::Type::PASS);
}

// ─────────────────────────────────────────────────────────────────
// getAction
// ─────────────────────────────────────────────────────────────────
PlayerAction
MafiaAgentProxy::getAction(const std::vector<Player> &players,
                           const std::vector<ChatMessage> &chat_history,
                           Phase current_phase,
                           const std::vector<std::string> &known_info) {
  knowledge_ = known_info; // Обновляем личные знания

  Json::Value request = buildRequest(players, chat_history, current_phase);

  Json::StreamWriterBuilder writer;
  writer["emitUTF8"] = true;
  writer["indentation"] = "";
  std::string json_request = Json::writeString(writer, request);

  std::string response = sendRequestToAPI("/mafia/agent_action", json_request);

  if (response.empty()) {
    return fallbackAction(players, current_phase, role_);
  }

  return parseActionFromJson(response, players, current_phase, role_);
}

// ─────────────────────────────────────────────────────────────────
// getChatMessage
// ─────────────────────────────────────────────────────────────────
std::string
MafiaAgentProxy::getChatMessage(const std::vector<Player> &players,
                                const std::vector<ChatMessage> &chat_history,
                                Phase current_phase,
                                const std::vector<std::string> &known_info) {
  knowledge_ = known_info;

  Json::Value request = buildRequest(players, chat_history, current_phase);

  Json::StreamWriterBuilder writer;
  writer["emitUTF8"] = true;
  writer["indentation"] = "";
  std::string json_request = Json::writeString(writer, request);

  std::string response = sendRequestToAPI("/mafia/agent_chat", json_request);

  if (response.empty()) {
    return ""; // Молчим при ошибке
  }

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
  try {
    std::string response = http_client_->post(api_url_ + endpoint, json_data);
    if (http_client_->getLastResponseCode() != 200) {
      std::cerr << "❌ API Error: " << http_client_->getLastResponseCode()
                << " for " << endpoint << std::endl;
      if (!response.empty()) {
        std::cerr << "Response: " << response.substr(0, 300) << std::endl;
      }
      return "";
    }
    return response;
  } catch (const std::exception &e) {
    std::cerr << "❌ API Exception: " << e.what() << std::endl;
    return "";
  }
}