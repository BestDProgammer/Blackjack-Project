cmake_minimum_required(VERSION 3.20)

project(BlackjackProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(BlackjackProject
    src/main.cpp
    src/Shoe.cpp
    src/SideBetEV.cpp
)

target_include_directories(BlackjackProject
    PRIVATE include
)