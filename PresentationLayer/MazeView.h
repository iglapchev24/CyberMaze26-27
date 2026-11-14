#pragma once
// =====================================================================================
//  PRESENTATION LAYER  -  MazeView
//  Draws the maze board (walls, numbers, trail, player, exit, doors).
//  (Owner: Frontend developer)
//
//  IDEAS FOR THE TEAM:  wall textures, particle trail, screen shake on a blocked move,
//  fog of war in extra-difficulty mode, nicer player sprite, sound effects.
// =====================================================================================
#include <imgui.h>
#include "../ApplicationLayer/Application.h"
using namespace std;

class MazeView {
public:
    // origin = top-left corner of the board on the screen, cellSize in pixels.
    static void draw(const GameEngine& gameEngine, ImDrawList* drawList, ImVec2 origin, float cellSize, double timeSeconds);
};
