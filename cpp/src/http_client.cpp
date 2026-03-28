#include "http_client.hpp"
#include <cstring>
#include <iostream>

// Callback для записи данных ответа
size_t HttpClient::WriteCallback(void *contents, size_t size, size_t nmemb,
                                 void *userp) {
  size_t total_size = size * nmemb;
  std::string *response = static_cast<std::string *>(userp);
  response->append(static_cast<char *>(contents), total_size);
  return total_size;
}

HttpClient::HttpClient() : curl_(nullptr), last_response_code_(0) {
  static bool is_global_initialized = false;
  if (!is_global_initialized) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    is_global_initialized = true;
  }
  curl_ = curl_easy_init();
}

HttpClient::~HttpClient() {
  if (curl_) {
    curl_easy_cleanup(curl_);
  }
  // curl_global_cleanup() намеренно не вызывается здесь,
  // так как другие экземпляры HttpClient могут продолжать работу.
}

std::string HttpClient::get(const std::string &url) {
  std::string response;

  if (!curl_) {
    return "";
  }

  curl_easy_reset(curl_); // Сбрасываем для нового запроса
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 300L);

  CURLcode res = curl_easy_perform(curl_);
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_response_code_);

  if (res != CURLE_OK) {
    return "";
  }

  return response;
}

std::string HttpClient::post(const std::string &url,
                             const std::string &json_body) {
  std::string response;

  if (!curl_) {
    return "";
  }

  // Устанавливаем опции для POST запроса
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_POST, 1L);
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_body.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT,
                   90L); // 90 секунд для надежности при работе с локальной LLM

  // Устанавливаем заголовки
  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

  CURLcode res = curl_easy_perform(curl_);

  curl_slist_free_all(headers);

  if (res != CURLE_OK) {
    return "";
  }

  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_response_code_);

  return response;
}
