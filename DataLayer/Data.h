#pragma once
// =====================================================================================
//  DATA LAYER  -  plain data types, constants and the dynamic 2D array.
//  No game rules and no drawing code live here.  (Owner: Backend developer)
// =====================================================================================
#include <cassert>
#include <string>
#include <utility>
using namespace std;

// ---------------------------------------------------------------------------------
//  Game constants (assignment: maze size is adjustable from 10x10 up to 50x50)
// ---------------------------------------------------------------------------------
const int MIN_MAZE_SIZE      = 10;
const int MAX_MAZE_SIZE      = 50;
const int DEFAULT_MAZE_SIZE  = 15;
const int MIN_CELL_COST      = 1;   // cheapest walkable square
const int MAX_CELL_COST      = 9;   // most expensive walkable square

// ---------------------------------------------------------------------------------
//  Enumerations
// ---------------------------------------------------------------------------------
enum class CellType {
    Wall,   // blocked square
    Path,   // normal walkable square (has a cost)
    Start,  // always the top-left square (0, 0), cost 0
    Exit,   // always the bottom-right square, cost 0
    Door    // special door: walkable "wall" that creates a shortcut to the exit
};

enum class GameState {
    MainMenu,   // nothing has been started yet - only the menu is shown
    Playing,    // the player is moving through the maze
    Victory     // the exit has been reached
};

enum class MoveResult {
    Rejected,       // wall, border, wrong state or an illegal diagonal
    Moved,          // the player moved to a new square
    ReachedExit     // the player moved onto the exit square
};

// ---------------------------------------------------------------------------------
//  Small structures
// ---------------------------------------------------------------------------------
struct GridPosition {
    int row = 0;
    int column = 0;
};

struct GameSettings {
    int  selectedMazeSize   = DEFAULT_MAZE_SIZE;  // value of the slider (10..50)
    bool extraDifficulty    = false;              // decoy paths / dead-end baits
};

// ---------------------------------------------------------------------------------
//  DynamicGrid<T>  -  a 2D array allocated on the heap with new[] / delete[].
//  Satisfies the assignment requirement "implementation with dynamic arrays".
//  It owns its memory (Rule of Five): copying makes a deep copy, no leaks.
// ---------------------------------------------------------------------------------
template <typename ElementType>
class DynamicGrid {
private:
    int rowCount = 0;
    int columnCount = 0;
    ElementType** rowPointers = nullptr;

    void allocate(int newRowCount, int newColumnCount) {
        if (newRowCount <= 0 || newColumnCount <= 0) {
            rowCount = 0;
            columnCount = 0;
            rowPointers = nullptr;
            return;
        }
        rowCount = newRowCount;
        columnCount = newColumnCount;
        rowPointers = new ElementType*[rowCount];
        for (int row = 0; row < rowCount; row++) {
            rowPointers[row] = new ElementType[columnCount];
        }
    }

    void release() {
        if (rowPointers != nullptr) {
            for (int row = 0; row < rowCount; row++) {
                delete[] rowPointers[row];
            }
            delete[] rowPointers;
        }
        rowPointers = nullptr;
        rowCount = 0;
        columnCount = 0;
    }

    void copyFrom(const DynamicGrid& other) {
        allocate(other.rowCount, other.columnCount);
        for (int row = 0; row < rowCount; row++) {
            for (int column = 0; column < columnCount; column++) {
                rowPointers[row][column] = other.rowPointers[row][column];
            }
        }
    }

public:
    DynamicGrid() = default;

    DynamicGrid(int newRowCount, int newColumnCount, const ElementType& initialValue) {
        resize(newRowCount, newColumnCount, initialValue);
    }

    DynamicGrid(const DynamicGrid& other) { copyFrom(other); }

    DynamicGrid& operator=(const DynamicGrid& other) {
        if (this != &other) {
            release();
            copyFrom(other);
        }
        return *this;
    }

    DynamicGrid(DynamicGrid&& other) noexcept
        : rowCount(other.rowCount), columnCount(other.columnCount), rowPointers(other.rowPointers) {
        other.rowCount = 0;
        other.columnCount = 0;
        other.rowPointers = nullptr;
    }

    DynamicGrid& operator=(DynamicGrid&& other) noexcept {
        if (this != &other) {
            release();
            rowCount = other.rowCount;
            columnCount = other.columnCount;
            rowPointers = other.rowPointers;
            other.rowCount = 0;
            other.columnCount = 0;
            other.rowPointers = nullptr;
        }
        return *this;
    }

    ~DynamicGrid() { release(); }

    // Throws the old content away and creates a new grid filled with initialValue.
    void resize(int newRowCount, int newColumnCount, const ElementType& initialValue) {
        release();
        allocate(newRowCount, newColumnCount);
        fill(initialValue);
    }

    void fill(const ElementType& value) {
        for (int row = 0; row < rowCount; row++) {
            for (int column = 0; column < columnCount; column++) {
                rowPointers[row][column] = value;
            }
        }
    }

    int getRowCount() const { return rowCount; }
    int getColumnCount() const { return columnCount; }

    bool isInside(int row, int column) const {
        return row >= 0 && row < rowCount && column >= 0 && column < columnCount;
    }

    ElementType& at(int row, int column) {
        assert(isInside(row, column));
        return rowPointers[row][column];
    }

    const ElementType& at(int row, int column) const {
        assert(isInside(row, column));
        return rowPointers[row][column];
    }
};

// ---------------------------------------------------------------------------------
//  MazeData  -  everything that describes one generated maze.
//  cellTypes[row][column] tells WHAT a square is, cellCosts[row][column] its value
//  (the number that is added to the score when the player steps on it).
// ---------------------------------------------------------------------------------
struct MazeData {
    int size = 0;                       // the maze is size x size squares
    DynamicGrid<CellType> cellTypes;
    DynamicGrid<int> cellCosts;
};

// Helper declared here, implemented in Data.cpp
bool isWalkable(CellType cellType);