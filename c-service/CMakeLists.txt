cmake_minimum_required(VERSION 3.20)
project(stockc LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(third_party/civetweb)
add_subdirectory(third_party/yyjson)

find_package(CURL REQUIRED)

add_executable(stockc
    src/main.c
    src/http_server.c
    src/http_client.c
    src/alpha_vantage.c
    src/routes/market.c
    src/cache/history_cache.c
    src/controllers/market_controller.c
    src/services/market_service.c
    src/services/market_metrics.c
    src/services/market_history_json.c
    src/services/market_demo_data.c
    src/http/cors.c
    src/http/responses.c
)

target_include_directories(stockc PRIVATE
    include
    third_party/civetweb/include
    third_party/yyjson/src
)

target_link_libraries(stockc
    civetweb-c-library
    yyjson
    CURL::libcurl
)