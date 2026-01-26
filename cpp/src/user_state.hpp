#ifndef USER_STATE_HPP
#define USER_STATE_HPP

#include <string>

struct UserState {
    std::string selected_game = "tic_tac_toe_5x5";
    std::string selected_agent = "random";
    std::string opponent_agent = "heuristic";
    int bet_amount = 100;
    int balance = 1000;
    std::string current_state = "main_menu";
};

#endif