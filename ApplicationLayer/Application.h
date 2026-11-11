#pragma once
#include "../DataLayer/Data.h"

class GameEngine {
private:
    GameState currentState = GameState::MainMenu;
    GameSettings gameSettings;
    PlayerPosition playerPosition;
    std::vector<std::vector<CellType>> mazeGrid;
    int mazeWidth = 15;
    int mazeHeight = 15;

public:
    GameEngine();

    void startNewGame();
    void resetToMenu();
    void movePlayer(int deltaX, int deltaY);

    // Getters
    GameState getCurrentState() const;
    const GameSettings& getGameSettings() const;
    const PlayerPosition& getPlayerPosition() const;
    const std::vector<std::vector<CellType>>& getMazeGrid() const;
    int getMazeWidth() const;
    int getMazeHeight() const;

private:
    void generateSampleMaze();
};