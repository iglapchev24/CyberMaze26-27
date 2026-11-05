cmake_minimum_required(VERSION 3.25)
project(DataLayer)

add_library(DataLayer STATIC
        Data.cpp
        Data.h
)

target_include_directories(DataLayer PUBLIC .)