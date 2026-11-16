#pragma once
// CYBERMAZE - raycast first-person visualization of the existing maze.
// No additional external libraries: rendered with Dear ImGui draw lists.
#include <imgui.h>
#include "../ApplicationLayer/Application.h"

namespace FirstPersonView {
    // Angle: zero looks east (+column), pi/2 looks south (+row).
    void draw(const GameEngine& engine, ImDrawList* drawList,
              ImVec2 topLeft, ImVec2 size, float viewAngle);
}
