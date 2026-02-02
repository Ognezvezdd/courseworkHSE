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
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl_ = curl_easy_init();
}

HttpClient::~HttpClient() {
  if (curl_) {
    curl_easy_cleanup(curl_);
  }
  curl_global_cleanup();
}

std::string HttpClient::get(const std::string &url) {
  std::string response;

  if (!curl_) {
    std::cerr << "CURL not initialized!" << std::endl;
    return "";
  }

  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);

  CURLcode res = curl_easy_perform(curl_);

  if (res != CURLE_OK) {
    std::cerr << "GET request failed: " << curl_easy_strerror(res) << std::endl;
    return "";
  }

  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_response_code_);

  return response;
}

std::string HttpClient::post(const std::string &url,
                             const std::string &json_body) {
  std::string response;

  if (!curl_) {
    std::cerr << "CURL not initialized!" << std::endl;
    return "";
  }

  // Устанавливаем опции для POST запроса
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_POST, 1L);
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_body.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 60L); // 60 секунд для игр

  // Устанавливаем заголовки
  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

  CURLcode res = curl_easy_perform(curl_);

  curl_slist_free_all(headers);

  if (res != CURLE_OK) {
    std::cerr << "POST request failed: " << curl_easy_strerror(res)
              << std::endl;
    return "";
  }

  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &last_response_code_);

  return response;
}
