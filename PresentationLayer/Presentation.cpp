#include "Presentation.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

bool ApplicationWindow::initializeWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // Разпъва прозореца на целия екран!

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(1280, 720, "CyberMaze26-27", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Тъмен кибернетичен стил
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.ItemSpacing = ImVec2(10, 10);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ApplicationWindow::renderMainMenu() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGui::Begin("MainMenuWindow", nullptr, windowFlags);

    // Центриране на менюто
    float windowWidth = ImGui::GetWindowSize().x;
    float windowHeight = ImGui::GetWindowSize().y;

    ImGui::SetCursorPosY(windowHeight * 0.25f);

    // Заглавие
    const char* titleText = "CYBER MAZE 26-27";
    float titleWidth = ImGui::CalcTextSize(titleText).x;
    ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
    ImGui::SetWindowFontScale(2.2f);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "%s", titleText);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Бутон START GAME
    float buttonWidth = 220.0f;
    float buttonHeight = 50.0f;
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.4f, 1.0f));
    if (ImGui::Button("START MISSION", ImVec2(buttonWidth, buttonHeight))) {
        gameEngine.startNewGame();
    }
    ImGui::PopStyleColor(2);

    // Бутон EXIT
    ImGui::Spacing();
    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("EXIT GAME", ImVec2(buttonWidth, buttonHeight))) {
        glfwSetWindowShouldClose(window, true);
    }
    ImGui::PopStyleColor();

    ImGui::End();
}

void ApplicationWindow::renderMazeGame() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::Begin("GameWindow", nullptr, flags);

    // Информационен панел отгоре
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "AGENT 007 ON MISSION");
    ImGui::SameLine(ImGui::GetWindowWidth() - 250);
    ImGui::Text("Coins: %d / %d", gameEngine.getGameSettings().coinsCollected, gameEngine.getGameSettings().totalCoinsInLevel);
    ImGui::Separator();

    // Изобразяване на лабиринта с бутони/квадрати
    const auto& grid = gameEngine.getMazeGrid();
    PlayerPosition playerPos = gameEngine.getPlayerPosition();

    float cellSize = 32.0f;
    float gridStartX = (ImGui::GetWindowWidth() - (gameEngine.getMazeWidth() * cellSize)) * 0.5f;
    float gridStartY = (ImGui::GetWindowHeight() - (gameEngine.getMazeHeight() * cellSize)) * 0.5f;

    ImGui::SetCursorPos(ImVec2(gridStartX, gridStartY));

    for (int y = 0; y < gameEngine.getMazeHeight(); y++) {
        for (int x = 0; x < gameEngine.getMazeWidth(); x++) {
            ImGui::SetCursorPos(ImVec2(gridStartX + x * cellSize, gridStartY + y * cellSize));

            ImVec4 cellColor = ImVec4(0.15f, 0.15f, 0.18f, 1.0f); // Път (празно)

            if (x == playerPos.x && y == playerPos.y) {
                cellColor = ImVec4(0.0f, 0.9f, 1.0f, 1.0f); // Играч (Син)
            } else {
                switch (grid[y][x]) {
                    case CellType::Wall: cellColor = ImVec4(0.3f, 0.3f, 0.35f, 1.0f); break; // Стена (Сиво)
                    case CellType::Exit: cellColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f); break; // Изход (Зелено)
                    case CellType::Coin: cellColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f); break; // Монета (Жълто)
                    default: break;
                }
            }

            ImGui::PushStyleColor(ImGuiCol_Button, cellColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, cellColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, cellColor);
            ImGui::Button("##cell", ImVec2(cellSize - 2, cellSize - 2));
            ImGui::PopStyleColor(3);
        }
    }

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
    ImGui::TextDisabled("Use WASD or Arrow Keys to move. Reach the GREEN exit!");

    ImGui::End();
}

void ApplicationWindow::renderVictoryScreen() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("VictoryWindow", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    float windowWidth = ImGui::GetWindowSize().x;
    ImGui::SetCursorPosY(ImGui::GetWindowSize().y * 0.3f);

    const char* victoryText = "MISSION ACCOMPLISHED!";
    ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(victoryText).x * 2.0f) * 0.5f);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "%s", victoryText);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::SetCursorPosX((windowWidth - 200) * 0.5f);
    ImGui::Text("Coins Collected: %d / %d", gameEngine.getGameSettings().coinsCollected, gameEngine.getGameSettings().totalCoinsInLevel);

    ImGui::Spacing();
    ImGui::SetCursorPosX((windowWidth - 200) * 0.5f);
    if (ImGui::Button("MAIN MENU", ImVec2(200, 45))) {
        gameEngine.resetToMenu();
    }

    ImGui::End();
}

void ApplicationWindow::handleInput() {
    if (gameEngine.getCurrentState() != GameState::Playing) return;

    static bool wPressed = false, aPressed = false, sPressed = false, dPressed = false;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (!wPressed) { gameEngine.movePlayer(0, -1); wPressed = true; }
    } else wPressed = false;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (!sPressed) { gameEngine.movePlayer(0, 1); sPressed = true; }
    } else sPressed = false;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!aPressed) { gameEngine.movePlayer(-1, 0); aPressed = true; }
    } else aPressed = false;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!dPressed) { gameEngine.movePlayer(1, 0); dPressed = true; }
    } else dPressed = false;
}

void ApplicationWindow::renderGui() {
    switch (gameEngine.getCurrentState()) {
        case GameState::MainMenu: renderMainMenu(); break;
        case GameState::Playing:  renderMazeGame(); break;
        case GameState::Victory:  renderVictoryScreen(); break;
    }
}

bool ApplicationWindow::run() {
    if (!initializeWindow()) return false;
    initializeImGui();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        handleInput();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderGui();

        ImGui::Render();
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    cleanup();
    return true;
}

void ApplicationWindow::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}