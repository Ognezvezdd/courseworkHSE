#include "mafia_game.hpp"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <random>
#include <sstream>

namespace Mafia {

MafiaGame::MafiaGame(int num_players)
    : current_phase_(Phase::NIGHT_MAFIA), current_day_(1), sheriff_id_(-1),
      doctor_id_(-1), don_id_(-1), game_over_(false), winner_("") {
  players_.reserve(num_players);
}

bool MafiaGame::initialize(
    const std::vector<std::shared_ptr<IMafiaAgent>> &agents) {
  if (agents.size() < 6) {
    game_log_.push_back("Ошибка: требуется минимум 6 игроков");
    return false;
  }

  agents_ = agents;

  // Создаем игроков с ролями, которые будут назначены позже
  for (size_t i = 0; i < agents_.size(); ++i) {
    if (!agents_[i]) {
      continue;
    }
    players_.emplace_back(i + 1, agents_[i]->getName(), Role::CITIZEN);
  }

  assignRoles();

  for (size_t i = 0; i < players_.size(); ++i) {
    std::string role_info = "Ваша роль: " + players_[i].getRoleName();
    players_[i].known_info.push_back(role_info);

    if (players_[i].isMafia()) {
      std::string mafia_team = "Ваша команда мафии: ";
      for (int mafia_id : getAliveMafiaIds()) {
        if (mafia_id != players_[i].id) {
          int idx = findPlayerById(mafia_id);
          if (idx != -1) {
            mafia_team += players_[idx].name + ", ";
          }
        }
      }
      if (mafia_team.length() > 20) {
        mafia_team = mafia_team.substr(0, mafia_team.length() - 2);
        players_[i].known_info.push_back(mafia_team);
      }
    }
  }

  // Инициализируем лог
  game_log_.push_back("=== ИГРА В МАФИЮ НАЧАЛАСЬ ===");
  game_log_.push_back("Количество игроков: " + std::to_string(players_.size()));
  game_log_.push_back("День 1, Ночь");

  // Первое сообщение в чат
  broadcastMessage("Игра началась! Наступает первая ночь...", true);

  return true;
}

void MafiaGame::assignRoles() {
  int num_players = players_.size();

  // Получаем распределение ролей
  std::vector<Role> roles = RoleUtils::getRoleDistribution(num_players);

  // Перемешиваем роли
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(roles.begin(), roles.end(), g);

  // Назначаем роли игрокам
  for (size_t i = 0; i < players_.size(); ++i) {
    players_[i].role = roles[i];

    // Запоминаем special roles
    if (roles[i] == Role::SHERIFF)
      sheriff_id_ = players_[i].id; // Store player ID, not index
    if (roles[i] == Role::DOCTOR)
      doctor_id_ = players_[i].id; // Store player ID, not index
    if (roles[i] == Role::DON)
      don_id_ = players_[i].id; // Store player ID, not index

    game_log_.push_back(players_[i].name + " - " + players_[i].getRoleName());
  }
}

bool MafiaGame::executeCycle() {
  if (game_over_)
    return false;

  // Ночная фаза
  current_phase_ = Phase::NIGHT_MAFIA;
  broadcastMessage("🌙 Ночь " + std::to_string(current_day_), true);
  executeNightPhase();

  if (game_over_)
    return false;

  // Дневная фаза
  current_phase_ = Phase::DAY_DISCUSSION;
  broadcastMessage("☀️ День " + std::to_string(current_day_), false);
  executeDayPhase();

  current_day_++;

  // Проверяем условия победы
  game_over_ = checkWinCondition();

  return !game_over_;
}

void MafiaGame::executeNightPhase() {
  night_actions_.clear();

  // 1. Мафия выбирает жертву
  current_phase_ = Phase::NIGHT_MAFIA;
  broadcastMessage("Мафия просыпается и выбирает жертву...", true);

  for (int mafia_id : getAliveMafiaIds()) {
    int idx = findPlayerById(mafia_id);
    if (idx == -1)
      continue;

    // Запрашиваем действие у агента-мафии
    PlayerAction action = agents_[idx]->getAction(
        players_, chat_history_, current_phase_, players_[idx].known_info);

    if (action.type == PlayerAction::Type::MAFIA_KILL &&
        action.target_id != -1) {
      night_actions_[mafia_id] = action;
    }
  }

  // 2. Дон проверяет (если жив)
  if (don_id_ != -1) {
    int don_idx = findPlayerById(don_id_);
    if (don_idx != -1 && players_[don_idx].is_alive) {
      current_phase_ = Phase::NIGHT_DON;
      sendPrivateMessage(don_id_, "Дон, кого вы хотите проверить?", true);

      PlayerAction action =
          agents_[don_idx]->getAction(players_, chat_history_, current_phase_,
                                      players_[don_idx].known_info);

      if (action.type == PlayerAction::Type::DON_CHECK &&
          action.target_id != -1) {
        night_actions_[don_id_] = action;
      }
    }
  }

  // 3. Шериф проверяет (если жив)
  if (sheriff_id_ != -1) {
    int sheriff_idx = findPlayerById(sheriff_id_);
    if (sheriff_idx != -1 && players_[sheriff_idx].is_alive) {
      current_phase_ = Phase::NIGHT_SHERIFF;
      sendPrivateMessage(sheriff_id_, "Шериф, кого вы хотите проверить?", true);

      PlayerAction action = agents_[sheriff_idx]->getAction(
          players_, chat_history_, current_phase_,
          players_[sheriff_idx].known_info);

      if (action.type == PlayerAction::Type::SHERIFF_CHECK &&
          action.target_id != -1) {
        night_actions_[sheriff_id_] = action;
      }
    }
  }

  // 4. Доктор лечит (если жив)
  if (doctor_id_ != -1) {
    int doctor_idx = findPlayerById(doctor_id_);
    if (doctor_idx != -1 && players_[doctor_idx].is_alive) {
      current_phase_ = Phase::NIGHT_DOCTOR;
      sendPrivateMessage(doctor_id_, "Доктор, кого вы хотите вылечить?", true);

      PlayerAction action = agents_[doctor_idx]->getAction(
          players_, chat_history_, current_phase_,
          players_[doctor_idx].known_info);

      if (action.type == PlayerAction::Type::DOCTOR_HEAL &&
          action.target_id != -1) {
        night_actions_[doctor_id_] = action;
      }
    }
  }

  // 5. Разрешаем ночные действия
  resolveNightActions();
}

void MafiaGame::resolveNightActions() {
  current_phase_ = Phase::NIGHT_RESULTS;

  // Собираем голоса мафии
  std::map<int, int> kill_votes; // target_id -> vote_count
  for (int mafia_id : getAliveMafiaIds()) {
    if (night_actions_.count(mafia_id)) {
      int target = night_actions_[mafia_id].target_id;
      kill_votes[target]++;
    }
  }

  // Находим цель с максимальным количеством голосов
  int kill_target = -1;
  int max_votes = 0;
  for (const auto &[target, votes] : kill_votes) {
    if (votes > max_votes && target != -1) {
      max_votes = votes;
      kill_target = target;
    }
  }

  // Проверяем защиту доктора
  bool is_protected = false;
  if (doctor_id_ != -1) {
    int doctor_idx = findPlayerById(doctor_id_);
    if (doctor_idx != -1 && players_[doctor_idx].is_alive &&
        night_actions_.count(doctor_id_)) {
      int heal_target = night_actions_[doctor_id_].target_id;
      if (heal_target == kill_target && kill_target != -1) {
        is_protected = true;
        int target_idx = findPlayerById(kill_target);
        if (target_idx != -1) {
          players_[target_idx].is_protected = true;
          sendPrivateMessage(
              doctor_id_,
              "Вы успешно защитили " + players_[target_idx].name + "!", true);
        }
      }
    }
  }

  // Выполняем убийство
  if (kill_target != -1 && !is_protected) {
    int target_idx = findPlayerById(kill_target);
    if (target_idx != -1) {
      killPlayer(kill_target, "убит мафией ночью");
      broadcastMessage("Ночью мафия убила " + players_[target_idx].name + "!",
                       false);
    }
  } else if (kill_target != -1 && is_protected) {
    broadcastMessage("Мафия пыталась убить кого-то, но доктор спас жертву!",
                     false);
  } else {
    broadcastMessage("Мафия не смогла договориться, никто не убит.", false);
  }

  // Обрабатываем проверку шерифа
  if (sheriff_id_ != -1) {
    int sheriff_idx = findPlayerById(sheriff_id_);
    if (sheriff_idx != -1 && players_[sheriff_idx].is_alive &&
        night_actions_.count(sheriff_id_)) {
      int check_target = night_actions_[sheriff_id_].target_id;
      int target_idx = findPlayerById(check_target);
      if (target_idx != -1) {
        bool is_mafia = players_[target_idx].isMafia();
        std::string result =
            is_mafia ? "является мафией" : "не является мафией";
        sendPrivateMessage(sheriff_id_,
                           players_[target_idx].name + " " + result, true);
        players_[sheriff_idx].known_info.push_back(
            "Проверка: " + players_[target_idx].name + " " + result);
      }
    }
  }

  // Обрабатываем проверку дона
  if (don_id_ != -1) {
    int don_idx = findPlayerById(don_id_);
    if (don_idx != -1 && players_[don_idx].is_alive &&
        night_actions_.count(don_id_)) {
      int check_target = night_actions_[don_id_].target_id;
      int target_idx = findPlayerById(check_target);
      if (target_idx != -1) {
        bool is_sheriff = players_[target_idx].role == Role::SHERIFF;
        std::string result =
            is_sheriff ? "является шерифом" : "не является шерифом";
        sendPrivateMessage(don_id_, players_[target_idx].name + " " + result,
                           true);
        players_[don_idx].known_info.push_back(
            "Проверка: " + players_[target_idx].name + " " + result);
      }
    }
  }

  // Сбрасываем защиту
  for (auto &player : players_) {
    player.is_protected = false;
  }
}

void MafiaGame::executeDayPhase() {
  // Фаза обсуждения
  current_phase_ = Phase::DAY_DISCUSSION;
  broadcastMessage(
      "Начинается дневное обсуждение. У вас есть время на дискуссию.", false);

  // Даем агентам возможность пообщаться
  for (int i = 0; i < 5; ++i) { // 5 раундов общения
    for (const auto &player : players_) {
      if (player.is_alive) {
        // agents_ vector is 0-indexed, player.id is 1-indexed
        std::string message = agents_[player.id - 1]->getChatMessage(
            players_, chat_history_, current_phase_, player.known_info);

        if (!message.empty()) {
          addChatMessage(player.id, message, false, true);
        }
      }
    }
  }

  // Фаза голосования
  current_phase_ = Phase::DAY_VOTING;
  broadcastMessage("Обсуждение завершено. Начинается голосование за изгнание.",
                   false);

  votes_.clear();

  // Собираем голоса
  for (const auto &player : players_) {
    if (player.is_alive) {
      // agents_ vector is 0-indexed, player.id is 1-indexed
      PlayerAction action = agents_[player.id - 1]->getAction(
          players_, chat_history_, current_phase_, player.known_info);

      if (action.type == PlayerAction::Type::VOTE_KILL &&
          action.target_id != -1) {
        int target_idx = findPlayerById(action.target_id);
        if (target_idx != -1 && players_[target_idx].is_alive) {
          votes_[action.target_id]++;
          players_[target_idx].votes_against++;

          addChatMessage(player.id,
                         "голосует против " + players_[target_idx].name, false,
                         true);
        }
      }
    }
  }

  // Разрешаем голосование
  resolveVoting();
}

void MafiaGame::resolveVoting() {
  current_phase_ = Phase::DAY_RESULTS;

  // Находим игрока с максимальным количеством голосов
  int exile_target = -1;
  int max_votes = 0;
  bool tie = false;

  for (const auto &[player_id, vote_count] : votes_) {
    if (vote_count > max_votes) {
      max_votes = vote_count;
      exile_target = player_id;
      tie = false;
    } else if (vote_count == max_votes && vote_count > 0) {
      tie = true;
    }
  }

  // Обрабатываем результат
  if (tie || max_votes == 0) {
    broadcastMessage("Голосование закончилось вничью. Никто не изгнан.", false);
  } else {
    int target_idx = findPlayerById(exile_target);
    if (target_idx != -1) {
      std::string role_reveal =
          " (роль: " + players_[target_idx].getRoleName() + ")";
      killPlayer(exile_target, "изгнан по результатам голосования");
      broadcastMessage(players_[target_idx].name + role_reveal +
                           " изгнан по результатам голосования!",
                       false);
    }
  }

  // Сбрасываем голоса
  for (auto &player : players_) {
    player.votes_against = 0;
  }
}

bool MafiaGame::checkWinCondition() {
  int alive_mafia = 0;
  int alive_citizens = 0;

  for (const auto &player : players_) {
    if (player.is_alive) {
      if (player.isMafia()) {
        alive_mafia++;
      } else {
        alive_citizens++;
      }
    }
  }

  // Условия победы
  if (alive_mafia == 0) {
    winner_ = "citizens";
    game_log_.push_back("Мирные жители победили!");
    broadcastMessage(
        "🎉 Мирные жители победили! Все мафиози найдены и наказаны.", false);
    return true;
  }

  if (alive_mafia >= alive_citizens) {
    winner_ = "mafia";
    game_log_.push_back("Мафия победила!");
    broadcastMessage("💀 Мафия победила! Мирные жители не смогли найти "
                     "всех преступников.",
                     false);
    return true;
  }

  return false;
}

void MafiaGame::killPlayer(int player_id, const std::string &reason) {
  int idx = findPlayerById(player_id);
  if (idx == -1)
    return;

  players_[idx].is_alive = false;
  game_log_.push_back(players_[idx].name + " " + reason);

  // Обновляем special IDs если нужно
  if (player_id == sheriff_id_)
    sheriff_id_ = -1;
  if (player_id == doctor_id_)
    doctor_id_ = -1;
  if (player_id == don_id_)
    don_id_ = -1;
}

void MafiaGame::addChatMessage(int player_id, const std::string &message,
                               bool is_night, bool is_public) {
  int idx = findPlayerById(player_id);
  if (idx == -1 || !players_[idx].is_alive)
    return;

  std::string timestamp = getCurrentTime();
  std::string role_name = players_[idx].getRoleName();

  ChatMessage msg(player_id, players_[idx].name, role_name, message, timestamp,
                  is_night, is_public);
  chat_history_.push_back(msg);

  // Добавляем в лог если это публичное сообщение
  if (is_public) {
    game_log_.push_back(msg.toString());
  }
}

void MafiaGame::broadcastMessage(const std::string &message, bool is_night) {
  std::string timestamp = getCurrentTime();
  ChatMessage msg(-1, "Система", "Системное", message, timestamp, is_night,
                  true);
  chat_history_.push_back(msg);
  game_log_.push_back(msg.toString());
}

void MafiaGame::sendPrivateMessage(int player_id, const std::string &message,
                                   bool is_night) {
  int idx = findPlayerById(player_id);
  if (idx == -1)
    return;

  std::string timestamp = getCurrentTime();
  ChatMessage msg(-1, "Система", "Системное", message, timestamp, is_night,
                  false);

  // Сохраняем в известной информации игрока
  players_[idx].known_info.push_back("[" + timestamp + "] " + message);
}

std::string MafiaGame::getCurrentTime() const {
  std::time_t t = std::time(nullptr);
  std::tm *tm_ptr = std::localtime(&t);
  if (!tm_ptr)
    return "00:00:00";

  std::ostringstream oss;
  oss << std::put_time(tm_ptr, "%H:%M:%S");
  return oss.str();
}

int MafiaGame::findPlayerById(int id) const {
  for (size_t i = 0; i < players_.size(); ++i) {
    if (players_[i].id == id) {
      return i;
    }
  }
  return -1;
}

std::vector<int> MafiaGame::getAlivePlayerIds() const {
  std::vector<int> alive;
  for (const auto &player : players_) {
    if (player.is_alive) {
      alive.push_back(player.id);
    }
  }
  return alive;
}

std::vector<int> MafiaGame::getAliveMafiaIds() const {
  std::vector<int> mafia;
  for (const auto &player : players_) {
    if (player.is_alive && player.isMafia()) {
      mafia.push_back(player.id);
    }
  }
  return mafia;
}

std::vector<int> MafiaGame::getAliveCitizenIds() const {
  std::vector<int> citizens;
  for (const auto &player : players_) {
    if (player.is_alive && player.isCitizen()) {
      citizens.push_back(player.id);
    }
  }
  return citizens;
}

MafiaResult MafiaGame::getResult() const {
  MafiaResult result;
  result.winner = winner_;
  result.total_days = current_day_;

  // Собираем команды
  for (const auto &player : players_) {
    if (player.isMafia()) {
      result.mafia_team.push_back(player.name + " (" + player.getRoleName() +
                                  ")");
    } else {
      result.citizen_team.push_back(player.name + " (" + player.getRoleName() +
                                    ")");
    }
  }

  // Собираем убитых игроков
  for (const auto &log_entry : game_log_) {
    if (log_entry.find("убит") != std::string::npos ||
        log_entry.find("изгнан") != std::string::npos) {
      result.killed_players.push_back(log_entry);
    }
  }

  result.chat_log = chat_history_;
  result.game_log = game_log_;

  // Считаем выживших
  result.surviving_players = 0;
  for (const auto &player : players_) {
    if (player.is_alive)
      result.surviving_players++;
  }

  return result;
}

// Реализация утилит для ролей
namespace RoleUtils {
std::string roleToString(Role role) {
  switch (role) {
  case Role::MAFIA:
    return "mafia";
  case Role::DON:
    return "don";
  case Role::SHERIFF:
    return "sheriff";
  case Role::DOCTOR:
    return "doctor";
  case Role::CITIZEN:
    return "citizen";
  default:
    return "unknown";
  }
}

Role stringToRole(const std::string &role_str) {
  if (role_str == "mafia")
    return Role::MAFIA;
  if (role_str == "don")
    return Role::DON;
  if (role_str == "sheriff")
    return Role::SHERIFF;
  if (role_str == "doctor")
    return Role::DOCTOR;
  if (role_str == "citizen")
    return Role::CITIZEN;
  return Role::CITIZEN; // По умолчанию мирный житель
}

bool isMafiaRole(Role role) { return role == Role::MAFIA || role == Role::DON; }

bool isCitizenRole(Role role) {
  return role == Role::CITIZEN || role == Role::SHERIFF || role == Role::DOCTOR;
}

std::string getRoleDescription(Role role) {
  switch (role) {
  case Role::MAFIA:
    return "Мафия: знает других мафиози, ночью голосует за убийство";
  case Role::DON:
    return "Дон мафии: знает мафию, может проверять игроков ночью";
  case Role::SHERIFF:
    return "Шериф: ночью может проверить игрока на причастность к мафии";
  case Role::DOCTOR:
    return "Доктор: ночью может защитить одного игрока от убийства";
  case Role::CITIZEN:
    return "Мирный житель: не имеет специальных способностей, голосует "
           "днем";
  default:
    return "Неизвестная роль";
  }
}

int getRecommendedMafiaCount(int total_players) {
  if (total_players <= 6)
    return 1;
  if (total_players <= 8)
    return 2;
  if (total_players <= 10)
    return 3;
  return 4; // Для очень больших игр
}

std::vector<Role> getRoleDistribution(int total_players) {
  std::vector<Role> roles;

  // Определяем количество мафии
  int mafia_count = getRecommendedMafiaCount(total_players);

  // Добавляем мафию
  for (int i = 0; i < mafia_count; ++i) {
    // Первая мафия - дон (если игроков достаточно)
    if (i == 0 && total_players >= 7) {
      roles.push_back(Role::DON);
    } else {
      roles.push_back(Role::MAFIA);
    }
  }

  // Добавляем шерифа (всегда)
  roles.push_back(Role::SHERIFF);

  // Добавляем доктора (если игроков достаточно)
  if (total_players >= 6) {
    roles.push_back(Role::DOCTOR);
  }

  // Остальные - мирные жители
  int remaining = total_players - roles.size();
  for (int i = 0; i < remaining; ++i) {
    roles.push_back(Role::CITIZEN);
  }

  return roles;
}
} // namespace RoleUtils

} // namespace Mafia