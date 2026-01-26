#include <iostream>
#include <curl/curl.h>
#include <string>

using namespace std;

// Функция для записи ответа
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    // ВАШ ТОКЕН БОТА (замените на реальный)
    string token = "8136569118:AAGkZK4wYGQULMDBB0q0S1t56HefH2Mjxj8";
    string base_url = "https://api.telegram.org/bot" + token + "/";
    
    cout << "=== Очистка очереди сообщений бота ===" << endl;
    cout << "Токен: " << token.substr(0, 10) << "..." << endl;
    cout << "Бот: https://t.me/" << token.substr(0, token.find(':')) << endl;
    
    // Инициализация CURL
    CURLcode curl_res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_res != CURLE_OK) {
        cerr << "Ошибка инициализации CURL: " << curl_easy_strerror(curl_res) << endl;
        return 1;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Не удалось создать CURL handle!" << endl;
        curl_global_cleanup();
        return 1;
    }
    
    // 1. Очистка всей очереди с offset=-1
    cout << "\n1. Очищаем всю очередь сообщений..." << endl;
    string url = base_url + "getUpdates?offset=-1";
    string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        cerr << "Ошибка при очистке: " << curl_easy_strerror(res) << endl;
    } else {
        cout << "✅ Очередь очищена!" << endl;
    }
    
    // 2. Проверяем текущее состояние
    cout << "\n2. Проверяем текущую очередь..." << endl;
    url = base_url + "getUpdates?limit=1";
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        if (response.find("\"result\":[]") != string::npos || 
            response.find("\"result\": []") != string::npos) {
            cout << "✅ Очередь пуста!" << endl;
        } else {
            cout << "⚠️ В очереди еще есть сообщения:" << endl;
            cout << response.substr(0, 300) << "..." << endl;
        }
    }
    
    // 3. Получаем информацию о боте для проверки
    cout << "\n3. Проверяем доступность бота..." << endl;
    url = base_url + "getMe";
    response.clear();
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        if (response.find("\"ok\":true") != string::npos) {
            cout << "✅ Бот доступен и работает!" << endl;
        } else {
            cout << "❌ Проблемы с ботом: " << endl;
            cout << response << endl;
        }
    }
    
    // Очистка
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    cout << "\n=== Очистка завершена ===" << endl;
    cout << "Теперь можно запускать бота командой: ./tg_bot" << endl;
    
    return 0;
}
