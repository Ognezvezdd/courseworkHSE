#ifndef USER_STATE_HPP
#define USER_STATE_HPP

#include <string>

struct UserState {
  std::string selected_game = "tic_tac_toe_5x5";
  std::string selected_agent = "";
  std::string opponent_agent = "heuristic";
  int bet_amount = 0;
  int balance = 1000;
  std::string current_state = "main_menu";
};

#endif