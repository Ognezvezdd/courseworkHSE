#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace Bunker {

// Перечисление профессий
enum class Profession {
    DOCTOR,
    ENGINEER,
    MILITARY,
    TEACHER,
    CHEF,
    STUDENT,
    HOMELESS,
    PILOT,
    FIREFIGHTER,
    POLICE,
    UNKNOWN
};

// Перечисление состояния здоровья
enum class Health {
    HEALTHY,
    SICK,
    INJURED,
    DISABLED
};

// Перечисление навыков
enum class Skill {
    MEDICINE,
    ENGINEERING,
    COMBAT,
    COOKING,
    TEACHING,
    PILOTING,
    NEGOTIATION,
    SURVIVAL
};

// Перечисление характеров
enum class Personality {
    CALM,
    AGGRESSIVE,
    PANICKER,
    CHARISMATIC,
    SELFISH,
    PARANOID
};

// Перечисление фаз игры
enum class GamePhase {
    WAITING,
    CHARACTER_CREATION,
    DISCUSSION,
    VOTING,
    EXILE,
    GAME_OVER
};

// Структура персонажа
struct PlayerCharacter {
    int player_id;
    std::string player_name;
    Profession profession;
    int age;
    Health health;
    std::vector<Skill> skills;
    Personality personality;
    std::optional<std::string> secret;
    std::vector<std::string> known_info;
    bool is_alive;
    bool is_exiled;
    
    PlayerCharacter() 
        : player_id(0), age(30), is_alive(true), is_exiled(false) {}
    
    std::string getProfessionString() const;
    std::string getHealthString() const;
    std::string getPersonalityString() const;
    std::string getSkillsString() const;
};

// Структура действия
struct BunkerAction {
    enum class Type {
        VOTE_EXILE,
        USE_SKILL,
        NEGOTIATE,
        PASS
    };
    
    Type type;
    int target_id;
    std::string message;
    
    BunkerAction(Type t = Type::PASS, int target = -1, const std::string& msg = "")
        : type(t), target_id(target), message(msg) {}
};

// Структура сообщения чата
struct ChatMessage {
    int player_id;
    std::string player_name;
    std::string text;
    std::string timestamp;
    bool is_public;
    
    ChatMessage(int id, const std::string& name, const std::string& msg, 
                const std::string& time, bool pub)
        : player_id(id), player_name(name), text(msg), 
          timestamp(time), is_public(pub) {}
    
    std::string toString() const;
};

// Результат игры
struct BunkerResult {
    std::string winner;  // "survive", "disaster", или "error"
    std::vector<std::string> survivors;
    std::vector<std::string> exiled_players;
    std::vector<std::string> game_log;
    std::vector<ChatMessage> chat_history;
    int total_rounds;
    int survivors_count;
};

// Утилиты для конвертации
namespace BunkerUtils {
    std::string professionToString(Profession p);
    Profession stringToProfession(const std::string& str);
    std::string healthToString(Health h);
    Health stringToHealth(const std::string& str);
    std::string skillToString(Skill s);
    Skill stringToSkill(const std::string& str);
    std::string personalityToString(Personality p);
    Personality stringToPersonality(const std::string& str);
    
    // Генерация случайных характеристик
    Profession getRandomProfession();
    Health getRandomHealth();
    Skill getRandomSkill();
    Personality getRandomPersonality();
    int getRandomAge();
}

} // namespace Bunker