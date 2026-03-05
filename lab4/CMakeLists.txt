cmake_minimum_required(VERSION 3.16)
project(lab4_stemmer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stem
    src/stemmer.cpp
    src/stem_main.cpp
)

target_include_directories(stem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)