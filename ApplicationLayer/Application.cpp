#include "Application.h"
#include <algorithm>
#include <climits>
#include <queue>
#include <vector>
using namespace std;

// ---------------------------------------------------------------------------------
//  Construction / destruction
// ---------------------------------------------------------------------------------
GameEngine::GameEngine() {
    highScoreSlotCount = (MAX_MAZE_SIZE - MIN_MAZE_SIZE + 1) * 2;
    highScores = new int[highScoreSlotCount];
    for (int slot = 0; slot < highScoreSlotCount; slot++) {
        highScores[slot] = -1;
    }
    buildNewMaze();   // a maze always exists, even before START is pressed
}

GameEngine::~GameEngine() {
    delete[] highScores;
}

// ---------------------------------------------------------------------------------
//  Private helpers
// ---------------------------------------------------------------------------------
void GameEngine::buildNewMaze() {
    generatedMazeSize = selectedSettings.selectedMazeSize;
    generatedExtraDifficulty = selectedSettings.extraDifficulty;
    mazeGenerator.generate(generatedMazeSize, generatedExtraDifficulty, currentMaze);
    optimalScore = computeOptimalScore();
    resetRun();
}

void GameEngine::resetRun() {
    visitedCells.resize(currentMaze.size, currentMaze.size, false);
    playerPosition = {0, 0};
    visitedCells.at(0, 0) = true;
    currentScore = 0;
    stepsTaken = 0;
    newHighScoreAchieved = false;
}

int GameEngine::getHighScoreSlot(int mazeSize, bool extraDifficulty) const {
    int clampedSize = max(MIN_MAZE_SIZE, min(MAX_MAZE_SIZE, mazeSize));
    return (clampedSize - MIN_MAZE_SIZE) * 2 + (extraDifficulty ? 1 : 0);
}

void GameEngine::updateHighScore() {
    int slot = getHighScoreSlot(generatedMazeSize, generatedExtraDifficulty);
    if (highScores[slot] == -1 || currentScore < highScores[slot]) {
        highScores[slot] = currentScore;
        newHighScoreAchieved = true;
    }
}

// Dijkstra: cheapest total of square values from the start to the exit,
// using the same movement rules as the player (8 directions).
int GameEngine::computeOptimalScore() const {
    int size = currentMaze.size;
    DynamicGrid<int> cheapestCost(size, size, INT_MAX);

    typedef pair<int, int> CostAndIndex;   // (cost so far, row * size + column)
    priority_queue<CostAndIndex, vector<CostAndIndex>, greater<CostAndIndex>> frontier;

    cheapestCost.at(0, 0) = 0;
    frontier.push({0, 0});

    while (!frontier.empty()) {
        CostAndIndex top = frontier.top();
        frontier.pop();
        int row = top.second / size;
        int column = top.second % size;
        if (top.first > cheapestCost.at(row, column)) {
            continue;   // outdated entry
        }
        for (int rowOffset = -1; rowOffset <= 1; rowOffset++) {
            for (int columnOffset = -1; columnOffset <= 1; columnOffset++) {
                if (!isMoveAllowed(currentMaze, {row, column}, rowOffset, columnOffset)) {
                    continue;
                }
                int nextRow = row + rowOffset;
                int nextColumn = column + columnOffset;
                int newCost = top.first + currentMaze.cellCosts.at(nextRow, nextColumn);
                if (newCost < cheapestCost.at(nextRow, nextColumn)) {
                    cheapestCost.at(nextRow, nextColumn) = newCost;
                    frontier.push({newCost, nextRow * size + nextColumn});
                }
            }
        }
    }
    return cheapestCost.at(size - 1, size - 1);
}

// ---------------------------------------------------------------------------------
//  Menu actions
// ---------------------------------------------------------------------------------
void GameEngine::setSelectedMazeSize(int requestedSize) {
    selectedSettings.selectedMazeSize = max(MIN_MAZE_SIZE, min(MAX_MAZE_SIZE, requestedSize));
}

void GameEngine::setSelectedExtraDifficulty(bool enabled) {
    selectedSettings.extraDifficulty = enabled;
}

// START (first time) or RESUME (game already running and settings untouched).
void GameEngine::startGame() {
    if (currentState == GameState::Playing && !haveSettingsChanged()) {
        return;   // resume: nothing to reset
    }
    if (haveSettingsChanged() || currentState == GameState::Victory) {
        buildNewMaze();
    } else {
        resetRun();
    }
    currentState = GameState::Playing;
}

void GameEngine::replayCurrentMaze() {
    if (currentState == GameState::MainMenu) {
        return;   // there is no played maze to replay yet
    }
    resetRun();
    currentState = GameState::Playing;
}

void GameEngine::generateNewMaze() {
    buildNewMaze();
    currentState = GameState::Playing;
}

// ---------------------------------------------------------------------------------
//  Gameplay
// ---------------------------------------------------------------------------------
// Rules: inside the maze, target is not a wall; a diagonal step may not squeeze
// between two walls (at least one of the two side squares must be open).
bool GameEngine::isMoveAllowed(const MazeData& maze, GridPosition from, int rowOffset, int columnOffset) {
    if (rowOffset < -1 || rowOffset > 1 || columnOffset < -1 || columnOffset > 1) {
        return false;
    }
    if (rowOffset == 0 && columnOffset == 0) {
        return false;
    }
    int targetRow = from.row + rowOffset;
    int targetColumn = from.column + columnOffset;
    if (!maze.cellTypes.isInside(targetRow, targetColumn) || !isWalkable(maze.cellTypes.at(targetRow, targetColumn))) {
        return false;
    }
    bool isDiagonal = rowOffset != 0 && columnOffset != 0;
    if (isDiagonal) {
        bool verticalSideOpen = isWalkable(maze.cellTypes.at(from.row + rowOffset, from.column));
        bool horizontalSideOpen = isWalkable(maze.cellTypes.at(from.row, from.column + columnOffset));
        if (!verticalSideOpen && !horizontalSideOpen) {
            return false;
        }
    }
    return true;
}

MoveResult GameEngine::movePlayer(int rowOffset, int columnOffset) {
    if (currentState != GameState::Playing) {
        return MoveResult::Rejected;
    }
    if (!isMoveAllowed(currentMaze, playerPosition, rowOffset, columnOffset)) {
        return MoveResult::Rejected;
    }

    playerPosition.row += rowOffset;
    playerPosition.column += columnOffset;
    visitedCells.at(playerPosition.row, playerPosition.column) = true;
    currentScore += currentMaze.cellCosts.at(playerPosition.row, playerPosition.column);
    stepsTaken++;

    if (currentMaze.cellTypes.at(playerPosition.row, playerPosition.column) == CellType::Exit) {
        currentState = GameState::Victory;
        updateHighScore();
        return MoveResult::ReachedExit;
    }
    return MoveResult::Moved;
}

// ---------------------------------------------------------------------------------
//  Getters
// ---------------------------------------------------------------------------------
GameState GameEngine::getCurrentState() const { return currentState; }
const MazeData& GameEngine::getMaze() const { return currentMaze; }
const DynamicGrid<bool>& GameEngine::getVisitedCells() const { return visitedCells; }
GridPosition GameEngine::getPlayerPosition() const { return playerPosition; }
int GameEngine::getCurrentCellCost() const { return currentMaze.cellCosts.at(playerPosition.row, playerPosition.column); }
int GameEngine::getCurrentScore() const { return currentScore; }
int GameEngine::getStepsTaken() const { return stepsTaken; }
int GameEngine::getOptimalScore() const { return optimalScore; }
bool GameEngine::isNewHighScore() const { return newHighScoreAchieved; }
const GameSettings& GameEngine::getSelectedSettings() const { return selectedSettings; }

int GameEngine::getHighScore(int mazeSize, bool extraDifficulty) const {
    return highScores[getHighScoreSlot(mazeSize, extraDifficulty)];
}

int GameEngine::getHighScoreOfCurrentMaze() const {
    return getHighScore(generatedMazeSize, generatedExtraDifficulty);
}

bool GameEngine::haveSettingsChanged() const {
    return selectedSettings.selectedMazeSize != generatedMazeSize ||
           selectedSettings.extraDifficulty != generatedExtraDifficulty;
}