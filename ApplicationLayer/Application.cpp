#include "Application.h"

GameEngine::GameEngine() {
    generateSampleMaze();
}

void GameEngine::generateSampleMaze() {
    // Ясно указваме CellType за всяка стойност в матрицата
    mazeGrid = {
        {CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall},
        {CellType::Wall, CellType::Start, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Coin, CellType::Empty, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Coin, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Wall, CellType::Coin, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Coin, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Coin, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty,CellType::Wall},
        {CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Coin, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Empty, CellType::Wall},
        {CellType::Wall, CellType::Coin, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Wall, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Empty, CellType::Exit, CellType::Wall},
        {CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall, CellType::Wall}
    };

    playerPosition = {1, 1};
    gameSettings.coinsCollected = 0;
    gameSettings.totalCoinsInLevel = 6;
}

void GameEngine::startNewGame() {
    generateSampleMaze();
    currentState = GameState::Playing;
}

void GameEngine::resetToMenu() {
    currentState = GameState::MainMenu;
}

void GameEngine::movePlayer(int deltaX, int deltaY) {
    if (currentState != GameState::Playing) return;

    int newX = playerPosition.x + deltaX;
    int newY = playerPosition.y + deltaY;

    // Проверка за граници и стени
    if (newX >= 0 && newX < mazeWidth && newY >= 0 && newY < mazeHeight) {
        CellType targetCell = mazeGrid[newY][newX];

        if (targetCell != CellType::Wall) {
            playerPosition.x = newX;
            playerPosition.y = newY;

            // Събиране на монета
            if (targetCell == CellType::Coin) {
                gameSettings.coinsCollected++;
                mazeGrid[newY][newX] = CellType::Empty;
            }
            // Достигане на изхода
            else if (targetCell == CellType::Exit) {
                currentState = GameState::Victory;
            }
        }
    }
}

GameState GameEngine::getCurrentState() const { return currentState; }
const GameSettings& GameEngine::getGameSettings() const { return gameSettings; }
const PlayerPosition& GameEngine::getPlayerPosition() const { return playerPosition; }
const std::vector<std::vector<CellType>>& GameEngine::getMazeGrid() const { return mazeGrid; }
int GameEngine::getMazeWidth() const { return mazeWidth; }
int GameEngine::getMazeHeight() const { return mazeHeight; }
