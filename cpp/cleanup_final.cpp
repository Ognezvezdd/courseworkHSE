#include <iostream>
#include <curl/curl.h>
#include <string>

using namespace std;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    string token = "8136569118:AAGkZK4wYGQULMDBB0q0S1t56HefH2Mjxj8";
    
    cout << "=== ФИНАЛЬНАЯ ОЧИСТКА ОЧЕРЕДИ ===" << endl;
    cout << "Бот: MLArenaForGamesBot (@MLArenaForGamesBot)" << endl;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    
    if (curl) {
        // 1. Получаем последний update_id
        cout << "\n1. Получаем последний update_id..." << endl;
        string url = "https://api.telegram.org/bot" + token + "/getUpdates?limit=1";
        string response;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK && response.find("update_id") != string::npos) {
            // Находим последний update_id
            size_t pos = response.find("\"update_id\":");
            if (pos != string::npos) {
                size_t start = pos + 11; // длина "\"update_id\":"
                size_t end = response.find(",", start);
                string last_id_str = response.substr(start, end - start);
                long last_id = stol(last_id_str);
                
                cout << "Последний update_id: " << last_id << endl;
                
                // 2. Пропускаем ВСЕ старые сообщения
                cout << "\n2. Пропускаем ВСЕ старые сообщения..." << endl;
                url = "https://api.telegram.org/bot" + token + "/getUpdates?offset=" + to_string(last_id + 1);
                response.clear();
                
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                res = curl_easy_perform(curl);
                
                if (res == CURLE_OK) {
                    cout << "✅ Все старые сообщения пропущены!" << endl;
                    cout << "Новый offset: " << (last_id + 1) << endl;
                }
            }
        }
        
        // 3. Проверяем, что очередь пуста
        cout << "\n3. Проверяем текущую очередь..." << endl;
        url = "https://api.telegram.org/bot" + token + "/getUpdates?limit=1";
        response.clear();
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            if (response.find("\"result\":[]") != string::npos) {
                cout << "✅ Очередь ПУСТА!" << endl;
            } else {
                cout << "Ответ: " << response.substr(0, 300) << endl;
            }
        }
        
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    
    cout << "\n=== ГОТОВО! ===" << endl;
    cout << "Теперь запускайте бота!" << endl;
    
    return 0;
}
