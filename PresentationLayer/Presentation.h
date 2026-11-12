#pragma once
// =====================================================================================
//  PRESENTATION LAYER  -  ApplicationWindow
//  Window creation, main loop, layout of the menu + game screen, keyboard input.
//  Talks to the game ONLY through GameEngine.            (Owner: Frontend developer)
// =====================================================================================
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <string>
#include "../ApplicationLayer/Application.h"
using namespace std;

// Bits for the four arrow keys (used to detect two keys pressed at the same time).
const int ARROW_UP    = 1;
const int ARROW_RIGHT = 2;
const int ARROW_DOWN  = 4;
const int ARROW_LEFT  = 8;

class ApplicationWindow {
private:
    GLFWwindow* window = nullptr;
    GameEngine gameEngine;

    // ---- layout / scrolling ----
    float menuContentHeight = 700.0f;   // measured every frame, used to centre the menu
    float gameSectionTop = 0.0f;        // Y where the game section starts (below the menu)
    float scrollTargetY = 0.0f;
    bool isScrollAnimating = false;
    bool scrollToGameRequested = false;
    bool isBoardInView = false;         // arrow keys only move the player when the board is visible

    // ---- input ----
    enum class InputPhase { Idle, Combining, Holding };
    InputPhase inputPhase = InputPhase::Idle;
    int combinedKeyMask = 0;
    int firedKeyMask = 0;
    double phaseStartTime = 0.0;
    double holdStartTime = 0.0;
    double lastMoveTime = 0.0;
    double blockedFlashUntil = 0.0;

    // ---- setup ----
    bool initializeWindow();
    void initializeImGui();
    void cleanup();

    // ---- input ----
    int readArrowKeyMask() const;
    void performMove(int keyMask, double nowSeconds);
    void handleInput();

    // ---- drawing ----
    void renderBackground(double timeSeconds);
    void renderGui();
    void renderMenuSection(float availableWidth, float viewportHeight);
    void renderGameSection(float availableWidth, float viewportHeight, double timeSeconds);
    void renderHud(float availableWidth);
    void renderVictoryPanel(ImVec2 boardOrigin, float boardPixels);
    void renderHowToPlayCard(float cardWidth);
    void updateSmoothScroll(float deltaSeconds);
    void requestScrollTo(float targetY);

public:
    bool run();
};