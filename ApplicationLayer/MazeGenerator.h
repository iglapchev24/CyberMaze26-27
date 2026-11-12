#pragma once
// =====================================================================================
//  APPLICATION LAYER  -  MazeGenerator
//  Builds a new random maze (walls, corridors, doors and the numbers on the squares).
//  NO cin / cout here (assignment requirement 10).      (Owner: Backend developer)
// =====================================================================================
#include <random>
#include "../DataLayer/Data.h"
using namespace std;

class MazeGenerator {
private:
    mt19937 randomEngine;

    // ---- random helpers ----
    int randomInRange(int minimumValue, int maximumValue);
    bool randomChance(int percentage);

    // ---- generation steps (called in this order by generate) ----
    void carveCorridors(MazeData& maze);                 // depth-first search "perfect" maze
    void connectExitForEvenSize(MazeData& maze);         // even sizes need one extra tail to the exit
    void removeDeadEnds(MazeData& maze, int percentage); // normal mode: fewer dead ends
    void placeShortcutDoors(MazeData& maze, int doorCount);
    void assignCellCosts(MazeData& maze, bool extraDifficulty);

    // ---- helpers ----
    int countWalkableNeighbours(const MazeData& maze, int row, int column) const;
    void computeStepDistances(const MazeData& maze, GridPosition source, DynamicGrid<int>& distances) const;
    void markShortestRoute(const MazeData& maze, DynamicGrid<bool>& isOnRoute) const;

public:
    MazeGenerator();

    // Fills `maze` with a brand new random maze of size x size squares.
    // size is clamped to MIN_MAZE_SIZE..MAX_MAZE_SIZE.
    void generate(int mazeSize, bool extraDifficulty, MazeData& maze);
};
