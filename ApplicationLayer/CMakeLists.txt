cmake_minimum_required(VERSION 3.25)
project(ApplicationLayer)

add_library(ApplicationLayer STATIC
        Application.cpp
        Application.h
        MazeGenerator.cpp
        MazeGenerator.h
)

target_include_directories(ApplicationLayer PUBLIC .)
target_link_libraries(ApplicationLayer PRIVATE DataLayer)