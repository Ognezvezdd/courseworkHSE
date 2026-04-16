#pragma once

#include "bunker_agent_interface.hpp"
#include "http_client.hpp"
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace Bunker {

// Локальный "прокси" агента: выбирает стратегию по имени.
// (На данный момент без обращения к Python API, чтобы игра собиралась/работала автономно.)
class BunkerAgentProxy : public IBunkerAgent {
public:
  BunkerAgentProxy(const std::string &name, const std::string &api_url);
  ~BunkerAgentProxy() override = default;

  std::string getName() const override { return name_; }

  BunkerAction getAction(const std::vector<PlayerCharacter> &players,
                         const std::vector<ChatMessage> &chat_history,
                         GamePhase current_phase,
                         const PlayerCharacter &my_character,
                         const std::vector<std::string> &known_info) override;

  std::string getChatMessage(const std::vector<PlayerCharacter> &players,
                             const std::vector<ChatMessage> &chat_history,
                             GamePhase current_phase,
                             const PlayerCharacter &my_character,
                             const std::vector<std::string> &known_info) override;

  void updateKnowledge(const std::string &info) override;
  void setCharacter(const PlayerCharacter &character) override;

private:
  int pickVoteTarget(const std::vector<PlayerCharacter> &players,
                     const PlayerCharacter &my_character);
  bool shouldUseApi() const;
  std::string sendRequestToAPI(const std::string &endpoint,
                               const std::string &json_data);

  std::string name_;
  std::string api_url_;
  PlayerCharacter my_character_;
  std::vector<std::string> knowledge_;
  std::mt19937 rng_;

  // оставляем поле, чтобы при желании позже подключить API как в мафии
  std::unique_ptr<HttpClient> http_client_;
};

} // namespace Bunker

