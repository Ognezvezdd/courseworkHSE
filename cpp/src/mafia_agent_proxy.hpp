#ifndef MAFIA_AGENT_PROXY_HPP
#define MAFIA_AGENT_PROXY_HPP

#include "mafia_game.hpp"
#include <string>
#include <random>
#include <algorithm>

namespace Mafia {

// Прокси-агент для связи с Python API
class MafiaAgentProxy : public IMafiaAgent {
public:
    MafiaAgentProxy(const std::string& name, const std::string& api_url);
    ~MafiaAgentProxy() override = default;
    
    // IMafiaAgent interface
    PlayerAction getAction(const std::vector<Player>& players,
                          const std::vector<ChatMessage>& chat_history,
                          Phase current_phase,
                          const std::vector<std::string>& known_info) override;
    
    std::string getChatMessage(const std::vector<Player>& players,
                              const std::vector<ChatMessage>& chat_history,
                              Phase current_phase) override;
    
    std::string getName() const override { return name_; }
    Role getRole() const override { return role_; }
    void updateKnowledge(const std::string& info) override;
    
    void setRole(Role role) { role_ = role; }
    
private:
    std::string name_;
    std::string api_url_;
    Role role_;
    std::vector<std::string> knowledge_;
    
    std::string sendRequestToAPI(const std::string& endpoint, 
                                 const std::string& json_data);
};

} // namespace Mafia

#endif // MAFIA_AGENT_PROXY_HPP