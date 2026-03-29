#include "bunker_game.hpp"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace Bunker {

BunkerGame::BunkerGame(int bunker_capacity)
    : current_phase_(GamePhase::WAITING)
    , current_round_(1)
    , bunker_capacity_(bunker_capacity)
    , disaster_timer_(5)
    , game_over_(false)
    , winner_("")
    , rng_(std::time(nullptr)) {
}

bool BunkerGame::initialize(const std::vector<std::shared_ptr<IBunkerAgent>>& agents) {
    if (agents.size() < 3) {
        addToLog("Ошибка: требуется минимум 3 игрока для игры в Бункер");
        return false;
    }
    
    if (agents.size() > bunker_capacity_ + 2) {
        addToLog("Ошибка: слишком много игроков");
        return false;
    }
    
    agents_ = agents;
    
    // Создаем персонажей для каждого агента
    for (size_t i = 0; i < agents_.size(); ++i) {
        if (!agents_[i]) continue;
        
        PlayerCharacter character = generateRandomCharacter(i + 1, agents_[i]->getName());
        players_.push_back(character);
        
        // Сообщаем агенту его характеристики
        agents_[i]->setCharacter(character);
        
        // Добавляем информацию о персонаже в known_info
        std::string info = "Ваш персонаж: " + character.getProfessionString() +
                          ", возраст " + std::to_string(character.age) +
                          ", здоровье: " + character.getHealthString();
        agents_[i]->updateKnowledge(info);
    }
    
    addToLog("=== ИГРА БУНКЕР НАЧАЛАСЬ ===");
    addToLog("Вместимость бункера: " + std::to_string(bunker_capacity_));
    addToLog("Количество игроков: " + std::to_string(players_.size()));
    addToLog("До катастрофы осталось: " + std::to_string(disaster_timer_) + " раундов");
    
    broadcastMessage("Игра началась! До катастрофы осталось " + 
                     std::to_string(disaster_timer_) + " раундов.");
    
    return true;
}

PlayerCharacter BunkerGame::generateRandomCharacter(int player_id, const std::string& player_name) {
    PlayerCharacter character;
    character.player_id = player_id;
    character.player_name = player_name;
    character.profession = BunkerUtils::getRandomProfession();
    character.age = BunkerUtils::getRandomAge();
    character.health = BunkerUtils::getRandomHealth();
    character.personality = BunkerUtils::getRandomPersonality();
    character.is_alive = true;
    character.is_exiled = false;
    
    // Генерируем 2-3 случайных навыка
    int num_skills = (rng_() % 2) + 2; // 2 или 3
    for (int i = 0; i < num_skills; ++i) {
        character.skills.push_back(BunkerUtils::getRandomSkill());
    }
    
    // Случайный секрет (20% шанс)
    std::vector<std::string> secrets = {
        "На самом деле я преступник в розыске",
        "У меня есть запас еды на месяц",
        "Я знаю, где находится секретный выход",
        "Я работаю на враждебную организацию"
    };
    
    if (rng_() % 5 == 0) { // 20% шанс
        character.secret = secrets[rng_() % secrets.size()];
    }
    
    return character;
}

bool BunkerGame::startGame() {
    if (players_.size() < 3) {
        return false;
    }
    
    current_phase_ = GamePhase::DISCUSSION;
    broadcastMessage("Обсуждение начинается! Вы можете общаться и убеждать других.");
    
    return true;
}

bool BunkerGame::executeCycle() {
    if (game_over_) return false;
    
    // Фаза обсуждения
    current_phase_ = GamePhase::DISCUSSION;
    broadcastMessage("📢 Раунд " + std::to_string(current_round_) + 
                    " - время для обсуждения!");
    
    executeDiscussionPhase();
    
    if (game_over_) return false;
    
    // Фаза голосования
    current_phase_ = GamePhase::VOTING;
    broadcastMessage("🗳️ Голосование начинается! Выберите, кого изгнать из бункера.");
    
    executeVotingPhase();
    
    if (game_over_) return false;
    
    // Переход к следующему раунду
    current_round_++;
    disaster_timer_--;
    
    if (disaster_timer_ <= 0) {
        broadcastMessage("💥 КАТАСТРОФА! Бункер закрывается!");
        game_over_ = checkWinCondition();
    } else {
        broadcastMessage("⏰ До катастрофы осталось " + std::to_string(disaster_timer_) + " раундов");
    }
    
    return !game_over_;
}

void BunkerGame::executeDiscussionPhase() {
    // Даем каждому живому игроку возможность высказаться
    for (const auto& player : players_) {
        if (!player.is_alive) continue;
        
        int idx = findPlayerIndex(player.player_id);
        if (idx == -1) continue;
        
        std::string message = agents_[idx]->getChatMessage(
            players_, chat_history_, current_phase_, 
            players_[idx], players_[idx].known_info
        );
        
        if (!message.empty()) {
            addChatMessage(player.player_id, message, true);
        }
    }
}

void BunkerGame::executeVotingPhase() {
    voting_results_.clear();
    
    // Собираем голоса
    for (const auto& player : players_) {
        if (!player.is_alive) continue;
        
        int idx = findPlayerIndex(player.player_id);
        if (idx == -1) continue;
        
        BunkerAction action = agents_[idx]->getAction(
            players_, chat_history_, current_phase_,
            players_[idx], players_[idx].known_info
        );
        
        if (action.type == BunkerAction::Type::VOTE_EXILE && action.target_id != -1) {
            int target_idx = findPlayerIndex(action.target_id);
            if (target_idx != -1 && players_[target_idx].is_alive) {
                voting_results_[action.target_id]++;
                addChatMessage(player.player_id, 
                    "голосует за изгнание " + players_[target_idx].player_name, true);
            }
        }
    }
    
    // Разрешаем голосование
    int exiled_id = resolveVoting();
    
    if (exiled_id != -1) {
        int idx = findPlayerIndex(exiled_id);
        if (idx != -1) {
            exilePlayer(exiled_id);
            broadcastMessage(players_[idx].player_name + 
                " был изгнан из бункера! (Профессия: " + 
                players_[idx].getProfessionString() + ")");
        }
    } else {
        broadcastMessage("Ничья в голосовании. Никто не изгнан.");
    }
}

int BunkerGame::resolveVoting() {
    if (voting_results_.empty()) {
        return -1;
    }
    
    // Находим максимальное количество голосов
    int max_votes = 0;
    for (const auto& [target, votes] : voting_results_) {
        if (votes > max_votes) {
            max_votes = votes;
        }
    }
    
    // Находим всех с максимальным количеством голосов
    std::vector<int> candidates;
    for (const auto& [target, votes] : voting_results_) {
        if (votes == max_votes) {
            candidates.push_back(target);
        }
    }
    
    // Если несколько кандидатов - ничья
    if (candidates.size() > 1) {
        return -1;
    }
    
    return candidates[0];
}

void BunkerGame::exilePlayer(int player_id) {
    int idx = findPlayerIndex(player_id);
    if (idx == -1) return;
    
    players_[idx].is_exiled = true;
    players_[idx].is_alive = false;
    addToLog(players_[idx].player_name + " был изгнан из бункера");
}

bool BunkerGame::checkWinCondition() {
    std::vector<int> alive_ids;
    for (const auto& player : players_) {
        if (player.is_alive) {
            alive_ids.push_back(player.player_id);
        }
    }
    
    int alive_count = alive_ids.size();
    
    // Если все помещаются в бункер - победа выживших
    if (alive_count <= bunker_capacity_) {
        winner_ = "survive";
        std::string survivors;
        for (const auto& player : players_) {
            if (player.is_alive) {
                survivors += player.player_name + " ";
            }
        }
        broadcastMessage("🏆 ПОБЕДА! Все выжившие (" + survivors + 
                        ") помещаются в бункер!");
        return true;
    }
    
    // Если катастрофа наступила
    if (disaster_timer_ <= 0) {
        winner_ = "disaster";
        
        // Случайно выбираем выживших
        std::shuffle(alive_ids.begin(), alive_ids.end(), rng_);
        
        std::string survivors;
        for (size_t i = 0; i < static_cast<size_t>(bunker_capacity_) && i < alive_ids.size(); ++i) {
            int idx = findPlayerIndex(alive_ids[i]);
            if (idx != -1) {
                survivors += players_[idx].player_name + " ";
            }
        }
        
        broadcastMessage("💥 КАТАСТРОФА! В бункере места только для " + 
                        std::to_string(bunker_capacity_) + 
                        ". Выжили: " + survivors);
        return true;
    }
    
    return false;
}

void BunkerGame::addChatMessage(int player_id, const std::string& message, bool is_public) {
    int idx = findPlayerIndex(player_id);
    if (idx == -1) return;
    
    std::string timestamp = getCurrentTime();
    ChatMessage msg(player_id, players_[idx].player_name, message, timestamp, is_public);
    chat_history_.push_back(msg);
    
    if (is_public) {
        addToLog(msg.toString());
    }
}

void BunkerGame::broadcastMessage(const std::string& message) {
    std::string timestamp = getCurrentTime();
    ChatMessage msg(-1, "Система", message, timestamp, true);
    chat_history_.push_back(msg);
    addToLog(msg.toString());
}

void BunkerGame::sendPrivateMessage(int player_id, const std::string& message) {
    int idx = findPlayerIndex(player_id);
    if (idx == -1) return;
    
    std::string timestamp = getCurrentTime();
    players_[idx].known_info.push_back("[" + timestamp + "] " + message);
}

std::string BunkerGame::getCurrentTime() const {
    std::time_t t = std::time(nullptr);
    std::tm* tm_ptr = std::localtime(&t);
    if (!tm_ptr) return "00:00:00";
    
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%H:%M:%S");
    return oss.str();
}

void BunkerGame::addToLog(const std::string& message) {
    game_log_.push_back(message);
    std::cout << "[Bunker] " << message << std::endl;
}

int BunkerGame::findPlayerIndex(int player_id) const {
    for (size_t i = 0; i < players_.size(); ++i) {
        if (players_[i].player_id == player_id) {
            return i;
        }
    }
    return -1;
}

std::vector<int> BunkerGame::getAlivePlayerIds() const {
    std::vector<int> alive;
    for (const auto& player : players_) {
        if (player.is_alive) {
            alive.push_back(player.player_id);
        }
    }
    return alive;
}

const PlayerCharacter* BunkerGame::getPlayerById(int player_id) const {
    int idx = findPlayerIndex(player_id);
    if (idx == -1) return nullptr;
    return &players_[idx];
}

BunkerResult BunkerGame::getResult() const {
    BunkerResult result;
    result.winner = winner_;
    result.total_rounds = current_round_;
    
    for (const auto& player : players_) {
        if (player.is_alive && !player.is_exiled) {
            result.survivors.push_back(player.player_name + " (" + 
                                      player.getProfessionString() + ")");
        }
        if (player.is_exiled) {
            result.exiled_players.push_back(player.player_name + " (" + 
                                           player.getProfessionString() + ")");
        }
    }
    
    result.survivors_count = result.survivors.size();
    result.game_log = game_log_;
    result.chat_history = chat_history_;
    
    return result;
}

// ==================== Утилиты для конвертации ====================

std::string PlayerCharacter::getProfessionString() const {
    return BunkerUtils::professionToString(profession);
}

std::string PlayerCharacter::getHealthString() const {
    return BunkerUtils::healthToString(health);
}

std::string PlayerCharacter::getPersonalityString() const {
    return BunkerUtils::personalityToString(personality);
}

std::string PlayerCharacter::getSkillsString() const {
    std::string result;
    for (size_t i = 0; i < skills.size(); ++i) {
        if (i > 0) result += ", ";
        result += BunkerUtils::skillToString(skills[i]);
    }
    return result;
}

std::string ChatMessage::toString() const {
    return "[" + timestamp + "] " + player_name + ": " + text;
}

// Реализация BunkerUtils
namespace BunkerUtils {

std::string professionToString(Profession p) {
    switch (p) {
        case Profession::DOCTOR: return "Врач";
        case Profession::ENGINEER: return "Инженер";
        case Profession::MILITARY: return "Военный";
        case Profession::TEACHER: return "Учитель";
        case Profession::CHEF: return "Повар";
        case Profession::STUDENT: return "Студент";
        case Profession::HOMELESS: return "Бездомный";
        case Profession::PILOT: return "Пилот";
        case Profession::FIREFIGHTER: return "Пожарный";
        case Profession::POLICE: return "Полицейский";
        default: return "Неизвестно";
    }
}

Profession getRandomProfession() {
    static std::vector<Profession> professions = {
        Profession::DOCTOR, Profession::ENGINEER, Profession::MILITARY,
        Profession::TEACHER, Profession::CHEF, Profession::STUDENT,
        Profession::HOMELESS, Profession::PILOT, Profession::FIREFIGHTER,
        Profession::POLICE
    };
    static std::mt19937 rng(std::time(nullptr));
    return professions[rng() % professions.size()];
}

Health getRandomHealth() {
    static std::vector<Health> healths = {
        Health::HEALTHY, Health::HEALTHY, Health::HEALTHY,  // 3/6 здоровы
        Health::SICK, Health::INJURED, Health::DISABLED
    };
    static std::mt19937 rng(std::time(nullptr));
    return healths[rng() % healths.size()];
}

Skill getRandomSkill() {
    static std::vector<Skill> skills = {
        Skill::MEDICINE, Skill::ENGINEERING, Skill::COMBAT,
        Skill::COOKING, Skill::TEACHING, Skill::PILOTING,
        Skill::NEGOTIATION, Skill::SURVIVAL
    };
    static std::mt19937 rng(std::time(nullptr));
    return skills[rng() % skills.size()];
}

Personality getRandomPersonality() {
    static std::vector<Personality> personalities = {
        Personality::CALM, Personality::AGGRESSIVE, Personality::PANICKER,
        Personality::CHARISMATIC, Personality::SELFISH, Personality::PARANOID
    };
    static std::mt19937 rng(std::time(nullptr));
    return personalities[rng() % personalities.size()];
}

int getRandomAge() {
    static std::mt19937 rng(std::time(nullptr));
    std::uniform_int_distribution<int> dist(18, 80);
    return dist(rng);
}

std::string healthToString(Health h) {
    switch (h) {
        case Health::HEALTHY: return "Здоров";
        case Health::SICK: return "Болен";
        case Health::INJURED: return "Ранен";
        case Health::DISABLED: return "Инвалид";
        default: return "Неизвестно";
    }
}

std::string skillToString(Skill s) {
    switch (s) {
        case Skill::MEDICINE: return "Медицина";
        case Skill::ENGINEERING: return "Инженерия";
        case Skill::COMBAT: return "Бой";
        case Skill::COOKING: return "Кулинария";
        case Skill::TEACHING: return "Обучение";
        case Skill::PILOTING: return "Пилотирование";
        case Skill::NEGOTIATION: return "Переговоры";
        case Skill::SURVIVAL: return "Выживание";
        default: return "Неизвестно";
    }
}

std::string personalityToString(Personality p) {
    switch (p) {
        case Personality::CALM: return "Спокойный";
        case Personality::AGGRESSIVE: return "Агрессивный";
        case Personality::PANICKER: return "Паникёр";
        case Personality::CHARISMATIC: return "Харизматичный";
        case Personality::SELFISH: return "Эгоистичный";
        case Personality::PARANOID: return "Параноик";
        default: return "Неизвестно";
    }
}

} // namespace BunkerUtils
} // namespace Bunker