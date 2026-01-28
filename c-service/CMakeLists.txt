cmake_minimum_required(VERSION 3.20)
project(stockc LANGUAGES C)

set(ENABLE_ASAN OFF CACHE BOOL "" FORCE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_CIVETWEB_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/civetweb EXCLUDE_FROM_ALL)
add_subdirectory(third_party/yyjson)

add_executable(stockc
    src/main.c
    src/http_server.c
    src/routes/market.c
)

target_include_directories(stockc PRIVATE
    include
    third_party/civetweb/include
    third_party/yyjson/src
)

target_link_libraries(stockc
    civetweb-c-library
    yyjson
)

if (MSVC)
    target_link_options(stockc PRIVATE /SUBSYSTEM:CONSOLE)
endif()
