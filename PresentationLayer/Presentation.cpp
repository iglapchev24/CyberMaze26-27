#include "Presentation.h"
#include "MazeView.h"
#include "FirstPersonView.h"
#include "Theme.h"
#include "UiWidgets.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
using namespace std;

// Timing of the keyboard handling (seconds)
static const double COMBINE_WINDOW_SECONDS  = 0.06;  // keys pressed within this time count as "together"
static const double REPEAT_DELAY_SECONDS    = 0.32;  // holding a key: wait before auto-repeat starts
static const double REPEAT_INTERVAL_SECONDS = 0.09;  // auto-repeat speed

// =====================================================================================
//  Setup
// =====================================================================================
bool ApplicationWindow::initializeWindow() {
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW!\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);   // the window fills the whole screen

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(1280, 720, "CyberMaze26-27", nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    return true;
}

void ApplicationWindow::initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;   // do not create imgui.ini next to the program
    // NOTE: keyboard navigation is NOT enabled on purpose - the arrow keys belong to the game.

    Theme::applyCyberStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ApplicationWindow::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// =====================================================================================
//  Keyboard input.
//  One arrow key  = straight move.   Two arrow keys pressed together = diagonal move.
//  Holding the same key(s) keeps moving (auto-repeat).
// =====================================================================================
int ApplicationWindow::readArrowKeyMask() const {
    int keyMask = 0;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    keyMask |= ARROW_UP;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) keyMask |= ARROW_RIGHT;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  keyMask |= ARROW_DOWN;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  keyMask |= ARROW_LEFT;
    return keyMask;
}

void ApplicationWindow::performMove(int keyMask, double nowSeconds) {
    int rowOffset = ((keyMask & ARROW_DOWN) ? 1 : 0) - ((keyMask & ARROW_UP) ? 1 : 0);
    int columnOffset = ((keyMask & ARROW_RIGHT) ? 1 : 0) - ((keyMask & ARROW_LEFT) ? 1 : 0);
    if (rowOffset == 0 && columnOffset == 0) {
        return;
    }
    if (gameEngine.movePlayer(rowOffset, columnOffset) == MoveResult::Rejected) {
        blockedFlashUntil = nowSeconds + 0.35;   // HUD shows "BLOCKED" for a moment
    }
}

// The camera only changes how we look around: the real player always moves on
// the original game grid, so score, blocked moves and victory remain unchanged.
void ApplicationWindow::resetFirstPersonCamera() {
    firstPersonAngle = 0.0f; // face east from the top-left start
    firstPersonLastStep = 0.0;
    firstPersonLastTurn = 0.0;
}

void ApplicationWindow::handleFirstPersonInput() {
    if (gameEngine.getCurrentState() != GameState::Playing || !isBoardInView || ImGui::GetIO().WantTextInput) return;
    const double now = glfwGetTime();
    const float TURN_STEP = 3.14159265358979323846f / 12.0f;
    const bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    if (now - firstPersonLastTurn >= 0.085) {
        if (left != right) {
            firstPersonAngle += right ? TURN_STEP : -TURN_STEP;
            firstPersonLastTurn = now;
        }
    }
    const bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    if (forward != backward && now - firstPersonLastStep >= 0.18) {
        // Grid-aligned steps: nearest of the four cardinal directions.
        const float direction = firstPersonAngle + (backward ? 3.14159265358979323846f : 0.0f);
        const float cosine = cosf(direction);
        const float sine = sinf(direction);
        const int rowStep = fabsf(sine) > fabsf(cosine) ? (sine > 0 ? 1 : -1) : 0;
        const int colStep = rowStep == 0 ? (cosine > 0 ? 1 : -1) : 0;
        if (gameEngine.movePlayer(rowStep, colStep) == MoveResult::Rejected)
            blockedFlashUntil = now + 0.35;
        firstPersonLastStep = now;
    }
}

void ApplicationWindow::handleInput() {
    if (firstPersonMode) {
        inputPhase = InputPhase::Idle;
        handleFirstPersonInput();
        return;
    }

    bool inputAllowed = gameEngine.getCurrentState() == GameState::Playing &&
                        isBoardInView && !ImGui::GetIO().WantTextInput;
    if (!inputAllowed) {
        inputPhase = InputPhase::Idle;
        return;
    }

    int keyMask = readArrowKeyMask();
    double nowSeconds = glfwGetTime();

    if (keyMask == 0) {
        inputPhase = InputPhase::Idle;
        return;
    }

    switch (inputPhase) {
        case InputPhase::Idle:
            inputPhase = InputPhase::Combining;
            combinedKeyMask = keyMask;
            phaseStartTime = nowSeconds;
            break;

        case InputPhase::Combining: {
            combinedKeyMask |= keyMask;
            bool hasVerticalKey = (combinedKeyMask & (ARROW_UP | ARROW_DOWN)) != 0;
            bool hasHorizontalKey = (combinedKeyMask & (ARROW_LEFT | ARROW_RIGHT)) != 0;
            bool isDiagonalCombo = hasVerticalKey && hasHorizontalKey;
            // Diagonal: move immediately. Single key: wait a few milliseconds for a second key.
            if (isDiagonalCombo || nowSeconds - phaseStartTime >= COMBINE_WINDOW_SECONDS) {
                performMove(combinedKeyMask, nowSeconds);
                firedKeyMask = combinedKeyMask;
                holdStartTime = nowSeconds;
                lastMoveTime = nowSeconds;
                inputPhase = InputPhase::Holding;
            }
            break;
        }

        case InputPhase::Holding: {
            bool pressedNewKey = (keyMask & ~firedKeyMask) != 0;
            if (pressedNewKey) {
                inputPhase = InputPhase::Combining;       // a new key = a new move
                combinedKeyMask = keyMask;
                phaseStartTime = nowSeconds;
            } else if (keyMask == firedKeyMask &&
                       nowSeconds - holdStartTime >= REPEAT_DELAY_SECONDS &&
                       nowSeconds - lastMoveTime >= REPEAT_INTERVAL_SECONDS) {
                performMove(firedKeyMask, nowSeconds);    // auto-repeat while held
                lastMoveTime = nowSeconds;
            }
            // If only SOME of the keys were released we simply wait until all are up.
            break;
        }
    }
}

// =====================================================================================
//  Drawing helpers
// =====================================================================================
void ApplicationWindow::requestScrollTo(float targetY) {
    scrollTargetY = targetY;
    isScrollAnimating = true;
}

// Eases the scroll position towards scrollTargetY. A mouse wheel turn cancels it.
void ApplicationWindow::updateSmoothScroll(float deltaSeconds) {
    if (scrollToGameRequested) {
        scrollToGameRequested = false;
        requestScrollTo(gameSectionTop);
    }
    if (!isScrollAnimating) {
        return;
    }
    if (ImGui::GetIO().MouseWheel != 0.0f) {
        isScrollAnimating = false;
        return;
    }
    float currentY = ImGui::GetScrollY();
    float difference = scrollTargetY - currentY;
    if (fabsf(difference) < 1.0f) {
        ImGui::SetScrollY(scrollTargetY);
        isScrollAnimating = false;
        return;
    }
    ImGui::SetScrollY(currentY + difference * min(1.0f, deltaSeconds * 9.0f));
}

// Dark gradient + a slowly moving neon grid, drawn behind everything.
void ApplicationWindow::renderBackground(double timeSeconds) {
    ImDrawList* backgroundList = ImGui::GetBackgroundDrawList();
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImU32 topColor = Theme::toColor(Theme::BackgroundTop);
    ImU32 bottomColor = Theme::toColor(Theme::BackgroundBottom);
    backgroundList->AddRectFilledMultiColor(ImVec2(0, 0), screenSize, topColor, topColor, bottomColor, bottomColor);

    float gridSpacing = 56.0f;
    float offset = fmodf((float)timeSeconds * 8.0f, gridSpacing);
    ImU32 lineColor = Theme::toColor(Theme::BackgroundGridLine);
    for (float x = -gridSpacing + offset; x < screenSize.x; x += gridSpacing) {
        backgroundList->AddLine(ImVec2(x, 0), ImVec2(x, screenSize.y), lineColor);
    }
    for (float y = -gridSpacing + offset; y < screenSize.y; y += gridSpacing) {
        backgroundList->AddLine(ImVec2(0, y), ImVec2(screenSize.x, y), lineColor);
    }
}

void ApplicationWindow::renderHowToPlayCard(float cardWidth) {
    UiWidgets::beginCard("HowToPlayCard", cardWidth, Theme::TextDim);

    ImGui::TextColored(Theme::CurrentScoreColor, "HOW TO PLAY");
    ImGui::Separator();

    const float labelColumn = 150.0f;
    struct HelpLine { const char* label; ImVec4 color; const char* text; };
    HelpLine helpLines[] = {
        {"OBJECTIVE",  Theme::StartButtonColor,  "Start in the top-left corner (S) and reach the exit (E) in the bottom-right corner. "
                                                 "Every square you step on adds its number to your SCORE. The LOWEST score wins!"},
        {"MOVE",       Theme::CurrentScoreColor, "Press ONE arrow key to move one square: UP, DOWN, LEFT or RIGHT. Hold the key to keep walking."},
        {"DIAGONAL",   Theme::NewMazeButtonColor,"Press TWO arrow keys AT THE SAME TIME, e.g. UP + RIGHT, to move diagonally. "
                                                 "You cannot squeeze diagonally between two walls."},
        {"DOORS",      Theme::DoorCellColor,     "Amber squares are special doors inside the walls. Walk through them to take a shortcut."},
        {"NUMBERS",    Theme::HighScoreColor,    "Green numbers are cheap, red numbers are expensive. Plan the cheapest route, not just the shortest."},
        {"EXTRA HARD", Theme::DangerColor,       "Adds decoys: dead ends full of tempting cheap numbers that lead nowhere. Walls and walking backwards still cost you!"},
    };
    for (const HelpLine& helpLine : helpLines) {
        ImGui::TextColored(helpLine.color, "%s", helpLine.label);
        ImGui::SameLine(labelColumn);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(helpLine.text);
        ImGui::PopTextWrapPos();
    }
    UiWidgets::endCard();
}

// =====================================================================================
//  MENU SECTION (top of the page)
// =====================================================================================
void ApplicationWindow::renderMenuSection(float availableWidth, float viewportHeight) {
    float columnWidth = min(760.0f, availableWidth - 40.0f);
    float columnLeft = (availableWidth - columnWidth) * 0.5f;
    float startY = max(24.0f, (viewportHeight - menuContentHeight) * 0.5f);   // vertical centring

    ImGui::SetCursorPos(ImVec2(columnLeft, startY));
    ImGui::BeginGroup();

    // ---- title ----
    UiWidgets::drawScaledText("CYBER MAZE", 4.0f, Theme::CurrentScoreColor, true, columnWidth, true);
    UiWidgets::drawScaledText("26-27  //  THE LOWEST SCORE WINS", 1.2f, Theme::TextDim, true, columnWidth);
    ImGui::Dummy(ImVec2(1, 6));

    // ---- settings card ----
    int selectedSize = gameEngine.getSelectedSettings().selectedMazeSize;
    bool extraDifficulty = gameEngine.getSelectedSettings().extraDifficulty;

    UiWidgets::beginCard("SettingsCard", columnWidth, Theme::CurrentScoreColor);
    float innerWidth = ImGui::GetContentRegionAvail().x;

    ImGui::TextColored(Theme::TextDim, "MAZE SIZE");
    ImGui::SameLine();
    ImGui::TextColored(Theme::CurrentScoreColor, "%d x %d", selectedSize, selectedSize);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 9));
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 24.0f);
    ImGui::SetNextItemWidth(innerWidth);
    // AlwaysClamp = input validation: typed values (Ctrl+click) are forced into 10..50.
    if (ImGui::SliderInt("##MazeSize", &selectedSize, MIN_MAZE_SIZE, MAX_MAZE_SIZE, "%d", ImGuiSliderFlags_AlwaysClamp)) {
        gameEngine.setSelectedMazeSize(selectedSize);
    }
    ImGui::PopStyleVar(2);

    ImGui::Dummy(ImVec2(1, 2));
    if (UiWidgets::toggleSwitch("##ExtraDifficulty", &extraDifficulty)) {
        gameEngine.setSelectedExtraDifficulty(extraDifficulty);
    }
    ImGui::SameLine();
    ImGui::TextColored(extraDifficulty ? Theme::DangerColor : Theme::TextNormal, "EXTRA DIFFICULTY");
    ImGui::SameLine();
    ImGui::TextColored(Theme::TextDim, extraDifficulty ? "(decoys and dead-end traps ON)" : "(off)");
    UiWidgets::endCard();

    // ---- buttons ----
    bool hasPlayedMaze = gameEngine.getCurrentState() != GameState::MainMenu;
    bool isResume = gameEngine.getCurrentState() == GameState::Playing && !gameEngine.haveSettingsChanged();
    float gap = ImGui::GetStyle().ItemSpacing.x;
    float halfWidth = (columnWidth - gap) * 0.5f;

    if (UiWidgets::glowButton(isResume ? "RESUME GAME" : "START GAME", ImVec2(columnWidth, 58), Theme::StartButtonColor)) {
        gameEngine.startGame();
        resetFirstPersonCamera();
        scrollToGameRequested = true;
    }
    if (UiWidgets::glowButton("REPLAY CURRENT MAZE", ImVec2(halfWidth, 46), Theme::ReplayButtonColor, hasPlayedMaze)) {
        gameEngine.replayCurrentMaze();
        resetFirstPersonCamera();
        scrollToGameRequested = true;
    }
    ImGui::SameLine();
    if (UiWidgets::glowButton("GENERATE NEW MAZE", ImVec2(halfWidth, 46), Theme::NewMazeButtonColor)) {
        gameEngine.generateNewMaze();
        resetFirstPersonCamera();
        scrollToGameRequested = true;
    }

    // ---- scores (each has its own colour) ----
    int highScore = gameEngine.getHighScore(selectedSize, extraDifficulty);
    string highScoreText = highScore < 0 ? "--" : to_string(highScore);
    string currentScoreText = to_string(gameEngine.getCurrentScore());
    UiWidgets::statChip("HIGH SCORE  (" + to_string(selectedSize) + "x" + to_string(selectedSize) +
                        (extraDifficulty ? " HARD" : " NORMAL") + ")", highScoreText, Theme::HighScoreColor, ImVec2(halfWidth, 72));
    ImGui::SameLine();
    UiWidgets::statChip("CURRENT SCORE", currentScoreText, Theme::CurrentScoreColor, ImVec2(halfWidth, 72));

    // ---- help ----
    renderHowToPlayCard(columnWidth);

    if (hasPlayedMaze) {
        UiWidgets::drawScaledText("scroll down to return to the maze", 1.0f, Theme::TextDim, true, columnWidth);
    }

    ImGui::EndGroup();
    menuContentHeight = ImGui::GetItemRectSize().y + 40.0f;
}

// =====================================================================================
//  GAME SECTION (below the menu)
// =====================================================================================
void ApplicationWindow::renderHud(float availableWidth) {
    const float chipHeight = 70.0f;
    const float chipWidth = 190.0f;
    GridPosition playerPosition = gameEngine.getPlayerPosition();

    int highScore = gameEngine.getHighScoreOfCurrentMaze();
    UiWidgets::statChip("CURRENT SCORE", to_string(gameEngine.getCurrentScore()), Theme::CurrentScoreColor, ImVec2(chipWidth, chipHeight));
    ImGui::SameLine();
    UiWidgets::statChip("HIGH SCORE", highScore < 0 ? "--" : to_string(highScore), Theme::HighScoreColor, ImVec2(chipWidth, chipHeight));
    ImGui::SameLine();
    UiWidgets::statChip("ROW , COL  |  SQUARE VALUE",
                        to_string(playerPosition.row) + "," + to_string(playerPosition.column) + "  |  " + to_string(gameEngine.getCurrentCellCost()),
                        Theme::TextNormal, ImVec2(chipWidth + 70, chipHeight));

    // buttons on the right side of the same row
    float buttonWidth = 150.0f;
    float rightEdge = availableWidth - 24.0f;
    ImGui::SameLine();
    ImGui::SetCursorPosX(max(ImGui::GetCursorPosX(), rightEdge - 3 * buttonWidth - 2 * ImGui::GetStyle().ItemSpacing.x));
    if (UiWidgets::glowButton("REPLAY", ImVec2(buttonWidth, chipHeight), Theme::ReplayButtonColor)) {
        gameEngine.replayCurrentMaze();
        resetFirstPersonCamera();
    }
    ImGui::SameLine();
    if (UiWidgets::glowButton("NEW MAZE", ImVec2(buttonWidth, chipHeight), Theme::NewMazeButtonColor)) {
        gameEngine.generateNewMaze();
        resetFirstPersonCamera();
    }
    ImGui::SameLine();
    if (UiWidgets::glowButton("^ MENU", ImVec2(buttonWidth, chipHeight), Theme::TextDim)) {
        requestScrollTo(0.0f);
    }

    // legend + status line
    ImGui::TextColored(Theme::StartCellColor, "S start");
    ImGui::SameLine(); ImGui::TextColored(Theme::ExitCellColor, "E exit");
    ImGui::SameLine(); ImGui::TextColored(Theme::DoorCellColor, "amber = door (shortcut)");
    ImGui::SameLine(); ImGui::TextColored(Theme::PlayerColor, "cyan = you");
    ImGui::SameLine(); ImGui::TextColored(Theme::TextDim, "|  arrows: move   2 arrows together: diagonal");
    // Switch 2D/3D without generating a new maze or resetting the score.
    // The view button has its own ID to avoid collisions with other controls.
    if (UiWidgets::glowButton(firstPersonMode ? "2D MAP##ModeSwitch" : "3D FIRST PERSON##ModeSwitch",
                              ImVec2(205, 34), Theme::CurrentScoreColor)) {
        firstPersonMode = !firstPersonMode;
        inputPhase = InputPhase::Idle;
    }
    ImGui::SameLine();
    ImGui::TextColored(Theme::TextDim, firstPersonMode
        ? "W/S or UP/DOWN: step   A/D or LEFT/RIGHT: turn   |  same maze and score"
        : "3D camera available - use the button on the left");
    if (glfwGetTime() < blockedFlashUntil) {
        ImGui::SameLine();
        ImGui::TextColored(Theme::DangerColor, "  BLOCKED!");
    }
}

void ApplicationWindow::renderVictoryPanel(ImVec2 boardOrigin, float boardPixels) {
    float panelWidth = min(540.0f, boardPixels - 20.0f);
    float panelHeight = 120.0f;
    ImVec2 panelMin(boardOrigin.x + (boardPixels - panelWidth) * 0.5f, boardOrigin.y + (boardPixels - panelHeight) * 0.5f);
    ImVec2 panelMax(panelMin.x + panelWidth, panelMin.y + panelHeight);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(panelMin, panelMax, Theme::toColor(ImVec4(0.02f, 0.04f, 0.09f, 0.94f)), 14.0f);
    drawList->AddRect(panelMin, panelMax, Theme::toColor(Theme::StartButtonColor), 14.0f, 0, 2.5f);

    int score = gameEngine.getCurrentScore();
    int optimal = gameEngine.getOptimalScore();

    ImGui::SetCursorScreenPos(ImVec2(panelMin.x, panelMin.y + 16));
    UiWidgets::drawScaledText("MISSION COMPLETE", 2.2f, Theme::StartButtonColor, true, panelWidth, true);
    UiWidgets::drawScaledText("YOUR SCORE  " + to_string(score), 1.7f, Theme::CurrentScoreColor, true, panelWidth);
    UiWidgets::drawScaledText("CHEAPEST POSSIBLE  " + to_string(optimal), 1.2f, Theme::TextNormal, true, panelWidth);

    if (score == optimal) {
        UiWidgets::drawScaledText("PERFECT ROUTE!", 1.4f, Theme::NewMazeButtonColor, true, panelWidth);
    } else if (gameEngine.isNewHighScore()) {
        UiWidgets::drawScaledText("NEW HIGH SCORE!", 1.4f, Theme::HighScoreColor, true, panelWidth);
    } else {
        UiWidgets::drawScaledText("try again for a lower score", 1.0f, Theme::TextDim, true, panelWidth);
    }
}

void ApplicationWindow::renderGameSection(float availableWidth, float viewportHeight, double timeSeconds) {
    const float sideMargin = 24.0f;
    const float hudHeight = 118.0f;

    ImGui::SetCursorPos(ImVec2(sideMargin, gameSectionTop + 14.0f));
    ImGui::BeginGroup();
    renderHud(availableWidth);
    ImGui::EndGroup();

    // The board gets ALL the remaining space of the screen (square, centred).
    float boardAreaWidth = availableWidth - 2.0f * sideMargin;
    float boardAreaHeight = viewportHeight - hudHeight - 24.0f;
    int mazeSize = gameEngine.getMaze().size;
    float cellSize = floorf(min(boardAreaWidth, boardAreaHeight) / (float)mazeSize);
    cellSize = max(cellSize, 6.0f);
    float boardPixels = cellSize * mazeSize;

    ImVec2 windowPosition = ImGui::GetWindowPos();
    float boardLeft = sideMargin + (boardAreaWidth - boardPixels) * 0.5f;
    float boardTopInWindow = gameSectionTop + hudHeight;
    ImVec2 boardOrigin(windowPosition.x + boardLeft, windowPosition.y + boardTopInWindow - ImGui::GetScrollY());

    if (firstPersonMode) {
        // Fill the available screen area with the raycast camera.
        const float viewLeft = sideMargin;
        const float viewTop = gameSectionTop + hudHeight + 14.0f;
        ImVec2 viewOrigin(windowPosition.x + viewLeft,
                          windowPosition.y + viewTop - ImGui::GetScrollY());
        ImVec2 viewSize(boardAreaWidth, max(160.0f, boardAreaHeight - 14.0f));
        FirstPersonView::draw(gameEngine, ImGui::GetWindowDrawList(), viewOrigin, viewSize, firstPersonAngle);
        if (gameEngine.getCurrentState() == GameState::Victory) {
            renderVictoryPanel(ImVec2(viewOrigin.x + (viewSize.x - min(viewSize.x, 640.0f)) * 0.5f,
                                      viewOrigin.y + (viewSize.y - min(viewSize.y, 500.0f)) * 0.5f),
                               min(viewSize.x, 640.0f));
        }
    } else {
        MazeView::draw(gameEngine, ImGui::GetWindowDrawList(), boardOrigin, cellSize, timeSeconds);
        if (gameEngine.getCurrentState() == GameState::Victory)
            renderVictoryPanel(boardOrigin, boardPixels);
    }
}

// =====================================================================================
//  Whole screen: ONE full-size scrolling window  ->  [ MENU ]  then  [ GAME ] below it
// =====================================================================================
void ApplicationWindow::renderGui() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;   // scroll bar stays enabled
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("CyberMazeRoot", nullptr, windowFlags);

    float viewportHeight = viewport->WorkSize.y;
    float availableWidth = ImGui::GetContentRegionAvail().x;   // already without the scroll bar

    renderMenuSection(availableWidth, viewportHeight);
    gameSectionTop = max(viewportHeight, menuContentHeight);

    if (gameEngine.getCurrentState() != GameState::MainMenu) {
        renderGameSection(availableWidth, viewportHeight, ImGui::GetTime());
        ImGui::SetCursorPos(ImVec2(0, gameSectionTop + viewportHeight - 1.0f));   // defines the scrollable height
        ImGui::Dummy(ImVec2(1, 1));
    }

    isBoardInView = gameEngine.getCurrentState() != GameState::MainMenu &&
                    ImGui::GetScrollY() > gameSectionTop - viewportHeight * 0.5f;
    updateSmoothScroll(ImGui::GetIO().DeltaTime);

    ImGui::End();
    ImGui::PopStyleVar();
}

// =====================================================================================
//  Main loop
// =====================================================================================
bool ApplicationWindow::run() {
    if (!initializeWindow()) return false;
    initializeImGui();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        handleInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderBackground(ImGui::GetTime());
        renderGui();

        ImGui::Render();
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.02f, 0.03f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    cleanup();
    return true;
}