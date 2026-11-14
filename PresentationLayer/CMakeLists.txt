cmake_minimum_required(VERSION 3.25)
project(PresentationLayer)

add_executable(PresentationLayer
        main.cpp
        Presentation.h
        Presentation.cpp
        MazeView.cpp
        MazeView.h
)

target_link_libraries(PresentationLayer PRIVATE ApplicationLayer DataLayer imgui)