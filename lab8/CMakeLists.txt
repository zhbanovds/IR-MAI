cmake_minimum_required(VERSION 3.16)
project(lab8_boolean_search LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(search
    src/index_reader.cpp
    src/bool_ops.cpp
    src/query_parser.cpp
    src/search_main.cpp
)

target_include_directories(search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)