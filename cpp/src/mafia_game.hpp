#ifndef MAFIA_GAME_HPP
#define MAFIA_GAME_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Mafia {

// Роли в мафии (каждый игрок должен иметь одну из этих ролей)
enum class Role {
  MAFIA,   // Мафия (обычный мафиозо)
  DON,     // Дон мафии (видит проверки шерифа)
  SHERIFF, // Шериф (может проверять игроков ночью)
  DOCTOR,  // Доктор (может лечить одного игрока за ночь)
  CITIZEN  // Мирный житель (основная масса игроков)
};

// Действия, которые могут выполнять агенты
struct PlayerAction {
  enum class Type {
    VOTE_KILL,     // Голосовать за убийство днем
    MAFIA_KILL,    // Мафия выбирает жертву ночью
    DON_CHECK,     // Дон проверяет шерифа
    SHERIFF_CHECK, // Шериф проверяет игрока
    DOCTOR_HEAL,   // Доктор лечит игрока
    CHAT_MESSAGE,  // Отправка сообщения в чат
    PASS           // Пропуск действия
  };

  Type type;
  int target_id;       // ID цели действия (-1 если нет цели)
  std::string message; // Сообщение для чата

  PlayerAction(Type t = Type::PASS, int target = -1,
               const std::string &msg = "")
      : type(t), target_id(target), message(msg) {}
};

// Состояние игрока
struct Player {
  int id;
  std::string name;
  Role role;
  bool is_alive;
  bool is_protected; // Защищен доктором в эту ночь
  int votes_against; // Количество голосов против (обнуляется каждый день)
  std::vector<std::string> known_info; // Информация, известная игроку

  Player(int id, const std::string &name, Role role)
      : id(id), name(name), role(role), is_alive(true), is_protected(false),
        votes_against(0) {}

  // Методы для проверки роли
  bool isMafia() const { return role == Role::MAFIA || role == Role::DON; }
  bool isCitizen() const {
    return role == Role::CITIZEN || role == Role::DOCTOR ||
           role == Role::SHERIFF;
  }
  bool isSpecial() const {
    return role == Role::DOCTOR || role == Role::SHERIFF || role == Role::DON;
  }

  std::string getRoleName() const {
    switch (role) {
    case Role::MAFIA:
      return "Мафия";
    case Role::DON:
      return "Дон мафии";
    case Role::SHERIFF:
      return "Шериф";
    case Role::DOCTOR:
      return "Доктор";
    case Role::CITIZEN:
      return "Мирный житель";
    default:
      return "Неизвестно";
    }
  }
};

// Сообщение в чате
struct ChatMessage {
  int player_id;
  std::string player_name;
  std::string player_role; // Роль как строка для отображения
  std::string text;
  std::string timestamp;
  bool is_night;  // Ночное или дневное сообщение
  bool is_public; // Видно всем или только определенной команде

  ChatMessage(int pid, const std::string &name, const std::string &role,
              const std::string &txt, const std::string &time, bool night,
              bool pub = true)
      : player_id(pid), player_name(name), player_role(role), text(txt),
        timestamp(time), is_night(night), is_public(pub) {}

  std::string toString() const {
    std::string time_of_day = is_night ? "🌙" : "☀️";
    return "[" + timestamp + "] " + time_of_day + " " + player_name + " (" +
           player_role + "): " + text;
  }
};

// Фаза игры
enum class Phase {
  NIGHT_MAFIA,    // Ночь: мафия обсуждает и выбирает жертву
  NIGHT_DON,      // Ночь: дон проверяет игрока (если есть)
  NIGHT_SHERIFF,  // Ночь: шериф проверяет игрока
  NIGHT_DOCTOR,   // Ночь: доктор лечит игрока
  NIGHT_RESULTS,  // Результаты ночи
  DAY_DISCUSSION, // День: обсуждение и выдвижение подозрений
  DAY_VOTING,     // День: голосование за изгнание
  DAY_RESULTS,    // Результаты голосования
  GAME_END        // Игра завершена
};

// Результат игры для отправки в API
struct MafiaResult {
  std::string winner;                      // "mafia", "citizens"
  std::vector<std::string> mafia_team;     // Состав мафии
  std::vector<std::string> citizen_team;   // Состав мирных
  std::vector<std::string> killed_players; // Убитые игроки по дням
  std::vector<ChatMessage> chat_log;       // Полная история чата
  std::vector<std::string> game_log;       // Подробный лог игры
  int total_days;
  int surviving_players;
  std::string final_image_url; // URL итогового изображения
  std::string json_output;     // JSON для API
};

// Интерфейс для агента (будет реализован Python-частью)
class IMafiaAgent {
public:
  virtual ~IMafiaAgent() = default;

  // Получить действие от агента
  virtual PlayerAction
  getAction(const std::vector<Player> &players,
            const std::vector<ChatMessage> &chat_history, Phase current_phase,
            const std::vector<std::string> &known_info) = 0;

  // Отправить сообщение в чат (может быть вызвано агентом)
  virtual std::string
  getChatMessage(const std::vector<Player> &players,
                 const std::vector<ChatMessage> &chat_history,
                 Phase current_phase,
                 const std::vector<std::string> &known_info) = 0;

  // Получить имя агента
  virtual std::string getName() const = 0;

  // Получить роль агента (если уже известна)
  virtual Role getRole() const = 0;

  // Обновить знания агента
  virtual void updateKnowledge(const std::string &info) = 0;
};

// Основной класс игры "Мафия"
class MafiaGame {
public:
  MafiaGame(int num_players = 6);
  ~MafiaGame() = default;

  // Инициализация игры с агентами
  bool initialize(const std::vector<std::shared_ptr<IMafiaAgent>> &agents);

  // Выполнить один цикл игры (ночь+день)
  bool executeCycle();

  // Получить текущее состояние игры в JSON
  std::string getGameStateJSON() const;

  // Получить состояние для конкретного игрока (с ограниченной информацией)
  std::string getPlayerStateJSON(int player_id) const;

  // Добавить сообщение в чат
  void addChatMessage(int player_id, const std::string &message, bool is_night,
                      bool is_public = true);

  // Получить историю чата
  const std::vector<ChatMessage> &getChatHistory() const {
    return chat_history_;
  }

  // Получить игроков
  const std::vector<Player> &getPlayers() const { return players_; }

  // Получить текущую фазу
  Phase getCurrentPhase() const { return current_phase_; }

  // Получить текущий день
  int getCurrentDay() const { return current_day_; }

  // Проверить, завершена ли игра
  bool isGameOver() const { return game_over_; }

  // Получить результат игры
  MafiaResult getResult() const;

  // Получить статистику
  std::map<std::string, int> getStatistics() const;

private:
  // Внутренние методы
  void assignRoles();
  void executeNightPhase();
  void executeDayPhase();
  void resolveNightActions();
  void resolveVoting();
  bool checkWinCondition();
  void killPlayer(int player_id, const std::string &reason);
  void executePlayerAction(const PlayerAction &action);
  std::string getCurrentTime() const;

  // Вспомогательные методы
  std::vector<int> getAlivePlayerIds() const;
  std::vector<int> getAliveMafiaIds() const;
  std::vector<int> getAliveCitizenIds() const;
  int findPlayerById(int id) const;
  void broadcastMessage(const std::string &message, bool is_night = false);
  void sendPrivateMessage(int player_id, const std::string &message,
                          bool is_night = false);

private:
  std::vector<Player> players_;
  std::vector<std::shared_ptr<IMafiaAgent>> agents_;
  std::vector<ChatMessage> chat_history_;
  std::vector<std::string> game_log_;

  Phase current_phase_;
  int current_day_;
  int sheriff_id_;
  int doctor_id_;
  int don_id_;
  bool game_over_;
  std::string winner_;

  // Текущие ночные действия
  std::map<int, PlayerAction> night_actions_; // player_id -> action
  std::map<int, int> votes_;                  // player_id -> votes count
};

// Утилиты для работы с ролями
namespace RoleUtils {
std::string roleToString(Role role);
Role stringToRole(const std::string &role_str);
bool isMafiaRole(Role role);
bool isCitizenRole(Role role);
std::string getRoleDescription(Role role);
int getRecommendedMafiaCount(int total_players);
std::vector<Role> getRoleDistribution(int total_players);
} // namespace RoleUtils

} // namespace Mafia

#endif // MAFIA_GAME_HPP