cmake_minimum_required(VERSION 3.25)
project(PresentationLayer)

add_executable(PresentationLayer
        main.cpp
        Presentation.h
        Presentation.cpp
)

target_link_libraries(PresentationLayer PRIVATE ApplicationLayer DataLayer)