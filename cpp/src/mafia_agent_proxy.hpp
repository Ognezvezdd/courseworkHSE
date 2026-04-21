#ifndef MAFIA_AGENT_PROXY_HPP
#define MAFIA_AGENT_PROXY_HPP

#include "mafia_game.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <random>
#include <string>

#include <json/json.h>
#include "http_client.hpp"

namespace Mafia {

// Прокси-агент для связи с Python API
class MafiaAgentProxy : public IMafiaAgent {
public:
  MafiaAgentProxy(const std::string &name, const std::string &api_url);
  ~MafiaAgentProxy() override = default;

  // IMafiaAgent interface
  PlayerAction getAction(const std::vector<Player> &players,
                         const std::vector<ChatMessage> &chat_history,
                         Phase current_phase,
                         const std::vector<std::string> &known_info) override;

  std::string
  getChatMessage(const std::vector<Player> &players,
                 const std::vector<ChatMessage> &chat_history,
                 Phase current_phase,
                 const std::vector<std::string> &known_info) override;

  std::string getName() const override { return name_; }
  Role getRole() const override { return role_; }
  void updateKnowledge(const std::string &info) override;

  void setRole(Role role) { role_ = role; }

private:
  std::string name_;
  std::string api_url_;
  Role role_;
  int my_player_id_ = -1;
  std::vector<std::string> knowledge_;
  std::unique_ptr<HttpClient> http_client_;

  // Расширенный контекст для LLM
  struct VotingRecord {
    int day;
    std::map<std::string, int> votes; // player_name -> vote_count
    std::string exiled;               // имя изгнанного (или "")
  };
  std::vector<VotingRecord> voting_history_;
  std::vector<std::string> eliminated_players_;           // Список выбывших
  std::vector<int> mafia_team_ids_;                      // ID союзников (только для мафии)
  std::map<int, std::string> known_roles_;               // Раскрытые роли player_id -> role

  std::string sendRequestToAPI(const std::string &endpoint,
                               const std::string &json_data);
  Json::Value buildRequest(const std::vector<Player> &players,
                           const std::vector<ChatMessage> &chat_history,
                           Phase current_phase) const;

public:
  void setPlayerId(int id) { my_player_id_ = id; }
  void addVotingRecord(int day, const std::map<std::string, int> &votes,
                       const std::string &exiled) {
    voting_history_.push_back({day, votes, exiled});
  }
  void addEliminatedPlayer(const std::string &player_info) {
    eliminated_players_.push_back(player_info);
  }
  void setMafiaTeam(const std::vector<int> &ids) { mafia_team_ids_ = ids; }
  void addKnownRole(int player_id, const std::string &role) {
    known_roles_[player_id] = role;
  }
};

} // namespace Mafia

#endif // MAFIA_AGENT_PROXY_HPP