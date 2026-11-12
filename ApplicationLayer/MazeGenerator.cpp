#include "MazeGenerator.h"
#include <algorithm>
#include <queue>
#include <vector>
using namespace std;

// Four orthogonal directions: up, right, down, left
static const int ROW_STEPS[4]    = {-1, 0, 1, 0};
static const int COLUMN_STEPS[4] = { 0, 1, 0, -1};

// A door must save at least this many steps, otherwise it is not worth placing.
static const int MIN_DOOR_STEP_SAVING = 6;

// Normal mode removes this percentage of the dead ends (creates loops = several routes).
static const int NORMAL_MODE_DEAD_END_REMOVAL_PERCENT = 65;

MazeGenerator::MazeGenerator() {
    random_device hardwareRandom;
    randomEngine.seed(hardwareRandom());   // different maze on every run (requirement 7)
}

int MazeGenerator::randomInRange(int minimumValue, int maximumValue) {
    uniform_int_distribution<int> distribution(minimumValue, maximumValue);
    return distribution(randomEngine);
}

bool MazeGenerator::randomChance(int percentage) {
    return randomInRange(1, 100) <= percentage;
}

// ---------------------------------------------------------------------------------
//  Public entry point
// ---------------------------------------------------------------------------------
void MazeGenerator::generate(int mazeSize, bool extraDifficulty, MazeData& maze) {
    mazeSize = max(MIN_MAZE_SIZE, min(MAX_MAZE_SIZE, mazeSize));   // validation

    maze.size = mazeSize;
    maze.cellTypes.resize(mazeSize, mazeSize, CellType::Wall);
    maze.cellCosts.resize(mazeSize, mazeSize, 0);

    carveCorridors(maze);
    connectExitForEvenSize(maze);

    maze.cellTypes.at(0, 0) = CellType::Start;
    maze.cellTypes.at(mazeSize - 1, mazeSize - 1) = CellType::Exit;

    if (!extraDifficulty) {
        // Easy: fewer dead ends, so there are usually several routes with different costs.
        removeDeadEnds(maze, NORMAL_MODE_DEAD_END_REMOVAL_PERCENT);
    }
    // Extra difficulty keeps EVERY dead end of the perfect maze (they become the decoys).

    int doorCount = extraDifficulty ? max(1, mazeSize / 12) : max(1, mazeSize / 8);
    placeShortcutDoors(maze, doorCount);

    assignCellCosts(maze, extraDifficulty);
}

// ---------------------------------------------------------------------------------
//  Step 1: iterative depth-first search on the "rooms" (even row / even column squares).
//  The squares between two rooms are carved to create a corridor.
// ---------------------------------------------------------------------------------
void MazeGenerator::carveCorridors(MazeData& maze) {
    int lastRoomIndex = (maze.size % 2 == 0) ? maze.size - 2 : maze.size - 1;

    vector<GridPosition> pendingRooms;
    maze.cellTypes.at(0, 0) = CellType::Path;
    pendingRooms.push_back({0, 0});

    while (!pendingRooms.empty()) {
        GridPosition currentRoom = pendingRooms.back();

        // Collect rooms two squares away that were not visited yet.
        vector<int> unvisitedDirections;
        for (int direction = 0; direction < 4; direction++) {
            int neighbourRow = currentRoom.row + ROW_STEPS[direction] * 2;
            int neighbourColumn = currentRoom.column + COLUMN_STEPS[direction] * 2;
            bool insideRoomArea = neighbourRow >= 0 && neighbourRow <= lastRoomIndex &&
                                  neighbourColumn >= 0 && neighbourColumn <= lastRoomIndex;
            if (insideRoomArea && maze.cellTypes.at(neighbourRow, neighbourColumn) == CellType::Wall) {
                unvisitedDirections.push_back(direction);
            }
        }

        if (unvisitedDirections.empty()) {
            pendingRooms.pop_back();   // dead end reached - backtrack
            continue;
        }

        int chosenDirection = unvisitedDirections[randomInRange(0, (int)unvisitedDirections.size() - 1)];
        int wallRow = currentRoom.row + ROW_STEPS[chosenDirection];
        int wallColumn = currentRoom.column + COLUMN_STEPS[chosenDirection];
        int nextRow = currentRoom.row + ROW_STEPS[chosenDirection] * 2;
        int nextColumn = currentRoom.column + COLUMN_STEPS[chosenDirection] * 2;

        maze.cellTypes.at(wallRow, wallColumn) = CellType::Path;
        maze.cellTypes.at(nextRow, nextColumn) = CellType::Path;
        pendingRooms.push_back({nextRow, nextColumn});
    }
}

// For an even size the last row/column is not part of the room grid,
// so we add a small tail:  room(size-2,size-2) -> (size-2,size-1) -> exit(size-1,size-1)
void MazeGenerator::connectExitForEvenSize(MazeData& maze) {
    if (maze.size % 2 != 0) {
        return;
    }
    maze.cellTypes.at(maze.size - 2, maze.size - 1) = CellType::Path;
    maze.cellTypes.at(maze.size - 1, maze.size - 1) = CellType::Path;
}

// ---------------------------------------------------------------------------------
//  Step 2 (normal mode only): open a wall next to a dead end so it joins another corridor.
// ---------------------------------------------------------------------------------
void MazeGenerator::removeDeadEnds(MazeData& maze, int percentage) {
    int lastIndex = maze.size - 1;

    for (int row = 0; row < maze.size; row++) {
        for (int column = 0; column < maze.size; column++) {
            CellType cellType = maze.cellTypes.at(row, column);
            bool isEndpoint = (row == 0 && column == 0) || (row == lastIndex && column == lastIndex);
            if (cellType != CellType::Path || isEndpoint) {
                continue;
            }
            if (countWalkableNeighbours(maze, row, column) != 1 || !randomChance(percentage)) {
                continue;
            }

            // Walls that separate this dead end from another open square.
            vector<int> openableDirections;
            for (int direction = 0; direction < 4; direction++) {
                int wallRow = row + ROW_STEPS[direction];
                int wallColumn = column + COLUMN_STEPS[direction];
                int beyondRow = row + ROW_STEPS[direction] * 2;
                int beyondColumn = column + COLUMN_STEPS[direction] * 2;
                if (maze.cellTypes.isInside(beyondRow, beyondColumn) &&
                    maze.cellTypes.at(wallRow, wallColumn) == CellType::Wall &&
                    isWalkable(maze.cellTypes.at(beyondRow, beyondColumn))) {
                    openableDirections.push_back(direction);
                }
            }
            if (!openableDirections.empty()) {
                int chosenDirection = openableDirections[randomInRange(0, (int)openableDirections.size() - 1)];
                maze.cellTypes.at(row + ROW_STEPS[chosenDirection], column + COLUMN_STEPS[chosenDirection]) = CellType::Path;
            }
        }
    }
}

// ---------------------------------------------------------------------------------
//  Step 3: doors.  A door is a wall square that CAN be walked through and that
//  shortens the way from the start to the exit by at least MIN_DOOR_STEP_SAVING steps.
// ---------------------------------------------------------------------------------
void MazeGenerator::placeShortcutDoors(MazeData& maze, int doorCount) {
    GridPosition startPosition = {0, 0};
    GridPosition exitPosition = {maze.size - 1, maze.size - 1};

    // All wall squares in random order.
    vector<GridPosition> wallCandidates;
    for (int row = 0; row < maze.size; row++) {
        for (int column = 0; column < maze.size; column++) {
            if (maze.cellTypes.at(row, column) == CellType::Wall) {
                wallCandidates.push_back({row, column});
            }
        }
    }
    shuffle(wallCandidates.begin(), wallCandidates.end(), randomEngine);

    DynamicGrid<int> distanceFromStart;
    DynamicGrid<int> distanceToExit;
    computeStepDistances(maze, startPosition, distanceFromStart);
    computeStepDistances(maze, exitPosition, distanceToExit);

    int placedDoors = 0;
    for (size_t index = 0; index < wallCandidates.size() && placedDoors < doorCount; index++) {
        int row = wallCandidates[index].row;
        int column = wallCandidates[index].column;

        // A door joins two squares that are on opposite sides of the wall.
        for (int axis = 0; axis < 2; axis++) {
            int sideARow = row + (axis == 0 ? -1 : 0);
            int sideAColumn = column + (axis == 0 ? 0 : -1);
            int sideBRow = row + (axis == 0 ? 1 : 0);
            int sideBColumn = column + (axis == 0 ? 0 : 1);

            if (!maze.cellTypes.isInside(sideARow, sideAColumn) || !maze.cellTypes.isInside(sideBRow, sideBColumn)) {
                continue;
            }
            if (!isWalkable(maze.cellTypes.at(sideARow, sideAColumn)) ||
                !isWalkable(maze.cellTypes.at(sideBRow, sideBColumn))) {
                continue;
            }

            int currentBestRoute = distanceFromStart.at(exitPosition.row, exitPosition.column);
            int routeViaDoorAB = distanceFromStart.at(sideARow, sideAColumn) + 2 + distanceToExit.at(sideBRow, sideBColumn);
            int routeViaDoorBA = distanceFromStart.at(sideBRow, sideBColumn) + 2 + distanceToExit.at(sideARow, sideAColumn);

            if (min(routeViaDoorAB, routeViaDoorBA) + MIN_DOOR_STEP_SAVING <= currentBestRoute) {
                maze.cellTypes.at(row, column) = CellType::Door;
                placedDoors++;
                // The maze changed, so the distances must be recalculated.
                computeStepDistances(maze, startPosition, distanceFromStart);
                computeStepDistances(maze, exitPosition, distanceToExit);
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------------
//  Step 4: the numbers on the squares.
//  Normal:  random 1..9 everywhere.
//  Extra difficulty:  the real (shortest) route gets HIGH numbers 4..9 while every
//  decoy square off the route gets cheap, tempting numbers 1..3.
// ---------------------------------------------------------------------------------
void MazeGenerator::assignCellCosts(MazeData& maze, bool extraDifficulty) {
    DynamicGrid<bool> isOnRoute(maze.size, maze.size, false);
    if (extraDifficulty) {
        markShortestRoute(maze, isOnRoute);
    }

    for (int row = 0; row < maze.size; row++) {
        for (int column = 0; column < maze.size; column++) {
            CellType cellType = maze.cellTypes.at(row, column);
            int cost = 0;

            if (cellType == CellType::Wall || cellType == CellType::Start || cellType == CellType::Exit) {
                cost = 0;
            } else if (!extraDifficulty) {
                cost = randomInRange(MIN_CELL_COST, MAX_CELL_COST);
            } else if (isOnRoute.at(row, column)) {
                cost = randomInRange(4, MAX_CELL_COST);
            } else {
                cost = randomInRange(MIN_CELL_COST, 3);
            }
            maze.cellCosts.at(row, column) = cost;
        }
    }
}

// ---------------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------------
int MazeGenerator::countWalkableNeighbours(const MazeData& maze, int row, int column) const {
    int walkableCount = 0;
    for (int direction = 0; direction < 4; direction++) {
        int neighbourRow = row + ROW_STEPS[direction];
        int neighbourColumn = column + COLUMN_STEPS[direction];
        if (maze.cellTypes.isInside(neighbourRow, neighbourColumn) &&
            isWalkable(maze.cellTypes.at(neighbourRow, neighbourColumn))) {
            walkableCount++;
        }
    }
    return walkableCount;
}

// Breadth-first search (4 directions). Unreachable squares keep the value -1.
void MazeGenerator::computeStepDistances(const MazeData& maze, GridPosition source, DynamicGrid<int>& distances) const {
    distances.resize(maze.size, maze.size, -1);

    queue<GridPosition> waitingSquares;
    distances.at(source.row, source.column) = 0;
    waitingSquares.push(source);

    while (!waitingSquares.empty()) {
        GridPosition current = waitingSquares.front();
        waitingSquares.pop();

        for (int direction = 0; direction < 4; direction++) {
            int nextRow = current.row + ROW_STEPS[direction];
            int nextColumn = current.column + COLUMN_STEPS[direction];
            if (maze.cellTypes.isInside(nextRow, nextColumn) &&
                isWalkable(maze.cellTypes.at(nextRow, nextColumn)) &&
                distances.at(nextRow, nextColumn) == -1) {
                distances.at(nextRow, nextColumn) = distances.at(current.row, current.column) + 1;
                waitingSquares.push({nextRow, nextColumn});
            }
        }
    }
}

// Marks every square of one shortest (fewest steps) route from the start to the exit.
void MazeGenerator::markShortestRoute(const MazeData& maze, DynamicGrid<bool>& isOnRoute) const {
    DynamicGrid<int> distanceToExit;
    computeStepDistances(maze, {maze.size - 1, maze.size - 1}, distanceToExit);

    GridPosition current = {0, 0};
    isOnRoute.at(current.row, current.column) = true;

    while (distanceToExit.at(current.row, current.column) > 0) {
        for (int direction = 0; direction < 4; direction++) {
            int nextRow = current.row + ROW_STEPS[direction];
            int nextColumn = current.column + COLUMN_STEPS[direction];
            if (distanceToExit.isInside(nextRow, nextColumn) &&
                distanceToExit.at(nextRow, nextColumn) == distanceToExit.at(current.row, current.column) - 1) {
                current = {nextRow, nextColumn};
                isOnRoute.at(current.row, current.column) = true;
                break;
            }
        }
    }
}