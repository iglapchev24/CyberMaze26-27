cmake_minimum_required(VERSION 4.3)
project(CyberMaze26_27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(DataLayer)
add_subdirectory(ApplicationLayer)
add_subdirectory(PresentationLayer)


add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        # If we use GLFW + OpenGL3 backend:
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
        PresentationLayer/main.cpp
        PresentationLayer/main.cpp
        PresentationLayer/main.cpp
)

target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)