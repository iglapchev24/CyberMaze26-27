// =====================================================================================
//  QA  -  automatic tests of the Data + Application layers (no window needed).
//  Build:  g++ -std=c++20 -Wall -Wextra QA/MazeEngineTests.cpp ApplicationLayer/*.cpp DataLayer/*.cpp -o tests
// =====================================================================================
#include <iostream>
#include "../ApplicationLayer/Application.h"
using namespace std;

static int passedTests = 0;
static int failedTests = 0;

static void check(bool condition, const string& testName) {
    if (condition) { passedTests++; }
    else { failedTests++; cout << "FAILED: " << testName << endl; }
}

// Walks the cheapest-step route by BFS to prove the exit is reachable (4 directions).
static bool isExitReachable(const MazeData& maze) {
    DynamicGrid<bool> seen(maze.size, maze.size, false);
    vector<GridPosition> stack = {{0, 0}};
    seen.at(0, 0) = true;
    int rowSteps[4] = {-1, 0, 1, 0}, columnSteps[4] = {0, 1, 0, -1};
    while (!stack.empty()) {
        GridPosition current = stack.back(); stack.pop_back();
        for (int d = 0; d < 4; d++) {
            int r = current.row + rowSteps[d], c = current.column + columnSteps[d];
            if (maze.cellTypes.isInside(r, c) && isWalkable(maze.cellTypes.at(r, c)) && !seen.at(r, c)) {
                seen.at(r, c) = true; stack.push_back({r, c});
            }
        }
    }
    return seen.at(maze.size - 1, maze.size - 1);
}

static void testGeneratedMazes() {
    MazeGenerator generator;
    for (int size = MIN_MAZE_SIZE; size <= MAX_MAZE_SIZE; size++) {
        for (int mode = 0; mode < 2; mode++) {
            MazeData maze;
            generator.generate(size, mode == 1, maze);
            string label = "size " + to_string(size) + (mode ? " hard" : " normal");
            check(maze.size == size, label + ": size stored");
            check(maze.cellTypes.at(0, 0) == CellType::Start, label + ": start at 0,0");
            check(maze.cellTypes.at(size - 1, size - 1) == CellType::Exit, label + ": exit bottom-right");
            check(isExitReachable(maze), label + ": exit reachable");
            bool costsValid = true;
            for (int r = 0; r < size; r++)
                for (int c = 0; c < size; c++) {
                    CellType t = maze.cellTypes.at(r, c);
                    int cost = maze.cellCosts.at(r, c);
                    bool special = (t == CellType::Wall || t == CellType::Start || t == CellType::Exit);
                    if (special && cost != 0) costsValid = false;
                    if (!special && (cost < MIN_CELL_COST || cost > MAX_CELL_COST)) costsValid = false;
                }
            check(costsValid, label + ": costs in range");
        }
    }
}

static void testSizeValidation() {
    GameEngine engine;
    engine.setSelectedMazeSize(3);   check(engine.getSelectedSettings().selectedMazeSize == MIN_MAZE_SIZE, "size 3 clamped to 10");
    engine.setSelectedMazeSize(999); check(engine.getSelectedSettings().selectedMazeSize == MAX_MAZE_SIZE, "size 999 clamped to 50");
    engine.setSelectedMazeSize(-7);  check(engine.getSelectedSettings().selectedMazeSize == MIN_MAZE_SIZE, "negative size clamped");
}

static void testMovementRules() {
    GameEngine engine;
    check(engine.movePlayer(1, 0) == MoveResult::Rejected, "no movement before START");
    engine.startGame();
    check(engine.getCurrentState() == GameState::Playing, "START begins the game");
    check(engine.movePlayer(-1, 0) == MoveResult::Rejected, "cannot leave maze upwards");
    check(engine.movePlayer(0, -1) == MoveResult::Rejected, "cannot leave maze to the left");
    check(engine.movePlayer(0, 0) == MoveResult::Rejected, "zero move rejected");
    check(engine.movePlayer(2, 0) == MoveResult::Rejected, "jump of 2 rejected");

    // Hand-made 3-wall test: diagonal squeeze between two walls is illegal.
    MazeData tiny;
    tiny.size = 3;
    tiny.cellTypes.resize(3, 3, CellType::Wall);
    tiny.cellCosts.resize(3, 3, 1);
    tiny.cellTypes.at(0, 0) = CellType::Start;
    tiny.cellTypes.at(1, 1) = CellType::Path;
    check(!GameEngine::isMoveAllowed(tiny, {0, 0}, 1, 1), "diagonal between two walls rejected");
    tiny.cellTypes.at(0, 1) = CellType::Path;
    check(GameEngine::isMoveAllowed(tiny, {0, 0}, 1, 1), "diagonal with one open side allowed");
    check(!GameEngine::isMoveAllowed(tiny, {0, 0}, 1, 0), "walking into a wall rejected");
}

static void testFullPlaythrough() {
    // Plays every maze with a simple greedy walk (depth-first with the engine's own rules).
    GameEngine engine;
    engine.setSelectedMazeSize(12);
    engine.generateNewMaze();
    const MazeData& maze = engine.getMaze();
    DynamicGrid<bool> seen(maze.size, maze.size, false);
    vector<GridPosition> path = {{0, 0}};
    seen.at(0, 0) = true;
    // iterative DFS storing the walk, replaying moves through movePlayer
    vector<GridPosition> walk;
    DynamicGrid<GridPosition> parent(maze.size, maze.size, GridPosition{-1, -1});
    vector<GridPosition> stack = {{0, 0}};
    int rs[4] = {-1, 0, 1, 0}, cs[4] = {0, 1, 0, -1};
    while (!stack.empty()) {
        GridPosition cur = stack.back(); stack.pop_back();
        for (int d = 0; d < 4; d++) {
            int r = cur.row + rs[d], c = cur.column + cs[d];
            if (maze.cellTypes.isInside(r, c) && isWalkable(maze.cellTypes.at(r, c)) && !seen.at(r, c)) {
                seen.at(r, c) = true; parent.at(r, c) = cur; stack.push_back({r, c});
            }
        }
    }
    GridPosition step = {maze.size - 1, maze.size - 1};
    while (!(step.row == 0 && step.column == 0)) { walk.push_back(step); step = parent.at(step.row, step.column); }
    int expectedScore = 0;
    MoveResult lastResult = MoveResult::Rejected;
    GridPosition at = {0, 0};
    for (int i = (int)walk.size() - 1; i >= 0; i--) {
        lastResult = engine.movePlayer(walk[i].row - at.row, walk[i].column - at.column);
        expectedScore += maze.cellCosts.at(walk[i].row, walk[i].column);
        at = walk[i];
    }
    check(lastResult == MoveResult::ReachedExit, "walking the route reaches the exit");
    check(engine.getCurrentState() == GameState::Victory, "state becomes Victory");
    check(engine.getCurrentScore() == expectedScore, "score = sum of stepped squares");
    check(engine.getCurrentScore() >= engine.getOptimalScore(), "score never below optimal");
    check(engine.getHighScoreOfCurrentMaze() == engine.getCurrentScore(), "high score stored");
    check(engine.movePlayer(0, 1) == MoveResult::Rejected, "no movement after victory");

    engine.replayCurrentMaze();
    check(engine.getCurrentScore() == 0 && engine.getStepsTaken() == 0, "replay resets score");
    check(engine.getCurrentState() == GameState::Playing, "replay returns to Playing");
}

int main() {
    testGeneratedMazes();
    testSizeValidation();
    testMovementRules();
    testFullPlaythrough();
    cout << "Passed: " << passedTests << "  Failed: " << failedTests << endl;
    return failedTests == 0 ? 0 : 1;
}
