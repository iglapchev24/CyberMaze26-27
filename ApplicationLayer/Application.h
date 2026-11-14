#pragma once
// =====================================================================================
//  APPLICATION LAYER  -  GameEngine
//  Game rules, movement, score and high scores.
//  NO cin / cout here (assignment requirement 10).      (Owner: Backend developer)
// =====================================================================================
#include "../DataLayer/Data.h"
#include "MazeGenerator.h"
using namespace std;

class GameEngine {
private:
    // ---- maze and player ----
    MazeData currentMaze;
    MazeGenerator mazeGenerator;
    DynamicGrid<bool> visitedCells;      // trail of squares the player has stepped on
    GridPosition playerPosition;

    // ---- state ----
    GameState currentState = GameState::MainMenu;
    GameSettings selectedSettings;       // what the menu currently shows (slider + toggle)
    int generatedMazeSize = DEFAULT_MAZE_SIZE;     // settings the CURRENT maze was built with
    bool generatedExtraDifficulty = false;

    // ---- score ----
    int currentScore = 0;                // sum of the values of all squares stepped on
    int stepsTaken = 0;
    int optimalScore = 0;                // cheapest possible route (Dijkstra)
    bool newHighScoreAchieved = false;

    // High scores: one slot per (size, difficulty). Dynamic array, -1 = no score yet.
    int* highScores = nullptr;
    int highScoreSlotCount = 0;

    // ---- private helpers ----
    void buildNewMaze();                 // generate + reset the run
    void resetRun();                     // player back to (0,0), score 0
    int computeOptimalScore() const;
    int getHighScoreSlot(int mazeSize, bool extraDifficulty) const;
    void updateHighScore();

public:
    GameEngine();
    ~GameEngine();
    GameEngine(const GameEngine&) = delete;              // owns a raw dynamic array
    GameEngine& operator=(const GameEngine&) = delete;

    // ---- menu actions ----
    void setSelectedMazeSize(int requestedSize);         // validated: clamped to 10..50
    void setSelectedExtraDifficulty(bool enabled);
    void startGame();                                    // START / RESUME button
    void replayCurrentMaze();                            // same maze, score back to 0
    void generateNewMaze();                              // new maze with the selected settings

    // ---- gameplay ----
    // rowOffset / columnOffset are each -1, 0 or +1. Diagonal = both non-zero.
    MoveResult movePlayer(int rowOffset, int columnOffset);

    // Pure rule check (used by movePlayer, the optimal-score search and the QA tests).
    static bool isMoveAllowed(const MazeData& maze, GridPosition from, int rowOffset, int columnOffset);

    // ---- getters for the presentation layer ----
    GameState getCurrentState() const;
    const MazeData& getMaze() const;
    const DynamicGrid<bool>& getVisitedCells() const;
    GridPosition getPlayerPosition() const;
    int getCurrentCellCost() const;
    int getCurrentScore() const;
    int getStepsTaken() const;
    int getOptimalScore() const;
    bool isNewHighScore() const;
    int getHighScore(int mazeSize, bool extraDifficulty) const;   // -1 = none yet
    int getHighScoreOfCurrentMaze() const;
    const GameSettings& getSelectedSettings() const;
    bool haveSettingsChanged() const;     // slider/toggle differ from the current maze
};