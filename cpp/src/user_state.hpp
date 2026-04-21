#ifndef USER_STATE_HPP
#define USER_STATE_HPP

#include <string>
#include <vector>

struct UserState {
  std::string selected_game;
  std::string selected_agent;
  std::string opponent_agent;

  // Для мафии
  std::string game_mode; // "tic_tac_toe" или "mafia"
  int mafia_players;
  std::vector<std::string> mafia_agents;

  // Для бункера
  int bunker_capacity;

  // Настройки
  std::string openai_api_key;
  bool is_waiting_for_api_key;

  UserState()
      : selected_game("tic_tac_toe_5x5"), selected_agent("random"),
        opponent_agent("random"), game_mode("tic_tac_toe"),
        mafia_players(6), bunker_capacity(4),
        is_waiting_for_api_key(false) {}
};

#endif // USER_STATE_HPP