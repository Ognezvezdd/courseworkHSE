cmake_minimum_required(VERSION 3.15)
project(TGBotHSE)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

add_compile_options(-Wall -Wextra -O2)

set(SOURCES
    TgBot/main.cpp
    TgBot/bot.cpp
    TgBot/keyboard.cpp
    src/config.cpp
    src/game_manager.cpp
    src/http_client.cpp
    src/mafia_game.cpp
    src/mafia_agent_proxy.cpp
    src/bunker_game.cpp
    src/bunker_agent_proxy.cpp
)

add_executable(tg_bot ${SOURCES})

target_include_directories(tg_bot PRIVATE 
    ${CURL_INCLUDE_DIRS} 
    ${JSONCPP_INCLUDE_DIRS}
    src
    TgBot
)

target_link_libraries(tg_bot 
    ${CURL_LIBRARIES} 
    ${JSONCPP_LIBRARIES}
    pthread
)