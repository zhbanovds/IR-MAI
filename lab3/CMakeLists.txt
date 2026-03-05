cmake_minimum_required(VERSION 3.16)
project(lab3_tokenizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tokenize
    src/tokenizer.cpp
    src/tokenize_main.cpp
)

target_include_directories(tokenize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)