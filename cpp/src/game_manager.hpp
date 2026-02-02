#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

#include "http_client.hpp"
#include <map>
#include <string>
#include <vector>

struct GameResult {
  std::string winner; // "X", "O", "draw", "error"
  int steps;
  int bet_amount;
  int win_amount;
  std::string image_url;      // URL изображения с визуализацией
  std::string image_filename; // Имя файла изображения
  std::string json_output;    // Результат игры в JSON
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
   * @brief Получить список доступных игр (на будущее)
   * @return Список игр
   */
  static std::vector<std::string> getAvailableGames();

  /**
   * @brief Проверить доступность API
   * @return true если API доступен
   */
  bool checkApiHealth();

private:
  std::string api_url_;
  HttpClient http_client_;

  /**
   * @brief Парсинг JSON ответа игры
   * @param json_response JSON строка
   * @return Структура GameResult
   */
  GameResult parseGameResponse(const std::string &json_response);
};

#endif
