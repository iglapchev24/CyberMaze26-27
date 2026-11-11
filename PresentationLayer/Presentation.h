#pragma once
#include <GLFW/glfw3.h>
#include "../ApplicationLayer/Application.h"

class ApplicationWindow {
private:
    GLFWwindow* window = nullptr;
    GameEngine gameEngine;

    bool initializeWindow();
    void initializeImGui();
    void renderGui();
    void renderMainMenu();
    void renderMazeGame();
    void renderVictoryScreen();
    void handleInput();
    void cleanup();

public:
    bool run();
};