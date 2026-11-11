#pragma once
#include <vector>
#include <string>

enum CellType {
    Empty = 0,
    Wall = 1,
    Start = 2,
    Exit = 3,
    Coin = 4
};

enum class GameState {
    MainMenu,
    Playing,
    Victory
};

struct PlayerPosition {
    int x = 1;
    int y = 1;
};

struct GameSettings {
    std::string playerTitle = "Agent Vanko";
    int currentLevel = 1;
    int coinsCollected = 0;
    int totalCoinsInLevel = 0;
};