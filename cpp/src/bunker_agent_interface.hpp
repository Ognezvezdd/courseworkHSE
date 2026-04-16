#pragma once

#include "bunker_models.hpp"
#include <vector>
#include <memory>

namespace Bunker {

// Интерфейс для AI агентов
class IBunkerAgent {
public:
    virtual ~IBunkerAgent() = default;
    
    // Получить имя агента
    virtual std::string getName() const = 0;
    
    // Получить действие (голосование, использование навыка)
    virtual BunkerAction getAction(
        const std::vector<PlayerCharacter>& players,
        const std::vector<ChatMessage>& chat_history,
        GamePhase current_phase,
        const PlayerCharacter& my_character,
        const std::vector<std::string>& known_info
    ) = 0;
    
    // Получить сообщение для чата
    virtual std::string getChatMessage(
        const std::vector<PlayerCharacter>& players,
        const std::vector<ChatMessage>& chat_history,
        GamePhase current_phase,
        const PlayerCharacter& my_character,
        const std::vector<std::string>& known_info
    ) = 0;
    
    // Обновить знания агента
    virtual void updateKnowledge(const std::string& info) = 0;
    
    // Установить роль/характеристики
    virtual void setCharacter(const PlayerCharacter& character) = 0;
};

} // namespace Bunker