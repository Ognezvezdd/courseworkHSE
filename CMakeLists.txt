cmake_minimum_required(VERSION 3.15)
project(TGBotHSE)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

add_compile_options(-Wall -Wextra -O2)

set(SOURCES
    src/main.cpp
    src/bot.cpp
)

add_executable(tg_bot ${SOURCES})

target_include_directories(tg_bot PRIVATE ${CURL_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(tg_bot ${CURL_LIBRARIES} ${JSONCPP_LIBRARIES})