#ifndef USER_STATE_HPP
#define USER_STATE_HPP

#include <string>
#include <vector>

struct UserState {
  std::string selected_game;
  std::string selected_agent;
  std::string opponent_agent;
  int bet_amount;

  // Для мафии
  std::string game_mode; // "tic_tac_toe", "mafia" или "bunker"
  int mafia_players;
  std::vector<std::string> mafia_agents;
  int bunker_capacity;

  UserState()
      : selected_game("tic_tac_toe_5x5"), selected_agent("random"),
        opponent_agent("random"), bet_amount(100), game_mode("tic_tac_toe"),
        mafia_players(6), bunker_capacity(4) {}
};

#endif // USER_STATE_HPP