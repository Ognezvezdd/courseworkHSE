#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <curl/curl.h>
#include <map>
#include <string>

/**
 * @brief Простой HTTP клиент на основе libcurl
 */
class HttpClient {
public:
  HttpClient();
  ~HttpClient();

  /**
   * @brief Выполнить GET запрос
   * @param url URL для запроса
   * @return Тело ответа
   */
  std::string get(const std::string &url);

  /**
   * @brief Выполнить POST запрос с JSON телом
   * @param url URL для запроса
   * @param json_body JSON строка для отправки
   * @return Тело ответа
   */
  std::string post(const std::string &url, const std::string &json_body);

  /**
   * @brief Получить код последнего HTTP ответа
   * @return HTTP код ответа
   */
  long getLastResponseCode() const { return last_response_code_; }

private:
  CURL *curl_;
  long last_response_code_;

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              void *userp);
};

#endif // HTTP_CLIENT_HPP
