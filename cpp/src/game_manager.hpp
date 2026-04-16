#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

#include "http_client.hpp"
#include "mafia_game.hpp"
#include "bunker_game.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

struct GameResult {
  std::string winner = "error";
  int steps = 0;
  std::string board_state = "";
  std::string image_url = "";
  std::string image_filename = "";
  std::string json_output = "";
  std::vector<std::string> game_log;
};

struct MafiaGameResult {
  std::string winner = "error";
  std::vector<std::string> mafia_team;
  std::vector<std::string> citizen_team;
  std::vector<std::string> killed_players;
  std::vector<Mafia::ChatMessage> chat_log;
  std::vector<std::string> game_log;
  int total_days = 0;
  int surviving_players = 0;
  std::string image_url = "";   // URL итогового изображения
  std::string json_output = ""; // JSON для API
};

struct BunkerGameResult {
  std::string winner = "error";   // "survive", "disaster", или "error"
  std::vector<std::string> survivors;
  std::vector<std::string> exiled_players;
  std::vector<Bunker::ChatMessage> chat_log;
  std::vector<std::string> game_log;
  int total_rounds = 0;
  int survivors_count = 0;
  int bet_amount = 0;
  int win_amount = 0;
  std::string json_output = "";
};

/**
 * @brief Менеджер игр - взаимодействие с Python API
 */
class GameManager {
public:
  /**
   * @brief Конструктор
   * @param api_url Базовый URL Python API (например, http://python-api:8000)
   */
  GameManager(const std::string &api_url);

  /**
   * @brief Запуск игры между двумя агентами через API
   * @param agent_x Тип агента для X
   * @param agent_o Тип агента для O
   * @param seed Seed для воспроизводимости (0 = случайный)
   * @return Результат игры
   */
  GameResult runGame(const std::string &agent_x, const std::string &agent_o,
                     int seed = 0);

  /**
   * @brief Запуск игры в мафию
   * @param agents Список агентов для игры (6-12 агентов)
   * @param num_players Количество игроков
   * @param use_chat Использовать ли чат между агентами
   * @return Результат игры в мафию
   */
  MafiaGameResult runMafiaGame(const std::vector<std::string> &agents,
                               int num_players = 6,
                               bool use_chat = true);

  /**
   * @brief Обучение агента через API
   * @param agent_type Тип агента (только qlearning)
   * @param episodes Количество эпизодов
   * @param seed Seed для воспроизводимости
   * @return true если успешно
   */
  bool trainAgent(const std::string &agent_type, int episodes = 1000,
                  int seed = 42);

  /**
   * @brief Получить список доступных агентов от API
   * @return Список агентов
   */
  std::vector<std::string> getAvailableAgents();

  /**
   * @brief Получить список доступных агентов для мафии
   * @return Список агентов
   */
  std::vector<std::string> getAvailableMafiaAgents();

  /**
   * @brief Получить список доступных игр
   * @return Список игр
   */
  static std::vector<std::string> getAvailableGames();

  /**
   * @brief Проверить доступность API
   * @return true если API доступен
   */
  bool checkApiHealth();

  /**
   * @brief Получить историю чата игры в мафию (для отладки)
   * @param game_id ID игры
   * @return История чата
   */
  std::vector<Mafia::ChatMessage>
  getMafiaChatHistory(const std::string &game_id);

  /**
   * @brief Запуск игры в Бункер
   * @param agents Список агентов для игры (минимум 3)
   * @param bunker_capacity Вместимость бункера (сколько может выжить)
   * @param bet_amount Размер ставки
   * @return Результат игры в Бункер
   */
  BunkerGameResult runBunkerGame(const std::vector<std::string>& agents,
                                 int bunker_capacity = 4,
                                 int bet_amount = 100);

  /**
   * @brief Получить список доступных агентов для Бункера
   * @return Список агентов
   */
  std::vector<std::string> getAvailableBunkerAgents();

private:
  std::string api_url_;
  HttpClient http_client_;

  /**
   * @brief Парсинг JSON ответа игры
   * @param json_response JSON строка
   * @return Структура GameResult
   */
  GameResult parseGameResponse(const std::string &json_response);

  /**
   * @brief Парсинг JSON ответа игры в мафию
   * @param json_response JSON строка
   * @return Структура MafiaGameResult
   */
  MafiaGameResult parseMafiaResponse(const std::string &json_response);

  /**
   * @brief Создать агентов для игры в мафию
   * @param agent_names Имена агентов
   * @return Вектор агентов
   */
  std::vector<std::shared_ptr<Mafia::IMafiaAgent>>
  createMafiaAgents(const std::vector<std::string> &agent_names);

  /**
   * @brief Создать агентов для игры в Бункер
   * @param agent_names Имена агентов
   * @return Вектор агентов
   */
  std::vector<std::shared_ptr<Bunker::IBunkerAgent>>
  createBunkerAgents(const std::vector<std::string>& agent_names);

  BunkerGameResult parseBunkerResponse(const std::string& json_response);
};

#endif // GAME_MANAGER_HPP