#include "MazeView.h"
#include "Theme.h"
#include <cmath>
#include <string>
using namespace std;

// Draws a centred label inside one square.
static void drawCellLabel(ImDrawList* drawList, ImVec2 cellMin, float cellSize, const string& label, const ImVec4& color) {
    float fontSize = max(8.0f, min(28.0f, cellSize * 0.55f));
    float scale = fontSize / ImGui::GetFontSize();
    ImVec2 baseSize = ImGui::CalcTextSize(label.c_str());
    ImVec2 position(cellMin.x + (cellSize - baseSize.x * scale) * 0.5f,
                    cellMin.y + (cellSize - baseSize.y * scale) * 0.5f);
    drawList->AddText(ImGui::GetFont(), fontSize, position, Theme::toColor(color), label.c_str());
}

void MazeView::draw(const GameEngine& gameEngine, ImDrawList* drawList, ImVec2 origin, float cellSize, double timeSeconds) {
    const MazeData& maze = gameEngine.getMaze();
    const DynamicGrid<bool>& visitedCells = gameEngine.getVisitedCells();
    GridPosition playerPosition = gameEngine.getPlayerPosition();
    float pulse = 0.5f + 0.5f * sinf((float)timeSeconds * 3.0f);
    float rounding = cellSize * 0.18f;

    // ---- board frame / glow ----
    float boardPixels = cellSize * maze.size;
    ImVec2 boardEnd(origin.x + boardPixels, origin.y + boardPixels);
    for (int layer = 3; layer >= 1; layer--) {
        drawList->AddRect(ImVec2(origin.x - layer * 3, origin.y - layer * 3), ImVec2(boardEnd.x + layer * 3, boardEnd.y + layer * 3),
                          Theme::toColor(Theme::withAlpha(Theme::PlayerColor, 0.08f)), 8.0f, 0, 3.0f);
    }
    drawList->AddRectFilled(origin, boardEnd, Theme::toColor(Theme::PathColor), 4.0f);

    // ---- squares ----
    for (int row = 0; row < maze.size; row++) {
        for (int column = 0; column < maze.size; column++) {
            ImVec2 cellMin(origin.x + column * cellSize, origin.y + row * cellSize);
            ImVec2 cellMax(cellMin.x + cellSize, cellMin.y + cellSize);
            ImVec2 innerMin(cellMin.x + 1, cellMin.y + 1);
            ImVec2 innerMax(cellMax.x - 1, cellMax.y - 1);
            CellType cellType = maze.cellTypes.at(row, column);
            int cost = maze.cellCosts.at(row, column);

            switch (cellType) {
                case CellType::Wall:
                    drawList->AddRectFilled(innerMin, innerMax, Theme::toColor(Theme::WallColor), rounding);
                    if (cellSize >= 12.0f) {
                        drawList->AddRect(innerMin, innerMax, Theme::toColor(Theme::withAlpha(Theme::WallEdgeColor, 0.45f)), rounding);
                    }
                    break;

                case CellType::Door:
                    drawList->AddRectFilled(innerMin, innerMax, Theme::toColor(Theme::withAlpha(Theme::DoorCellColor, 0.90f)), rounding);
                    drawCellLabel(drawList, cellMin, cellSize, to_string(cost), ImVec4(0.10f, 0.05f, 0.0f, 1.0f));
                    break;

                case CellType::Start:
                    drawList->AddRectFilled(innerMin, innerMax, Theme::toColor(Theme::withAlpha(Theme::StartCellColor, 0.35f)), rounding);
                    drawCellLabel(drawList, cellMin, cellSize, "S", Theme::StartCellColor);
                    break;

                case CellType::Exit:
                    drawList->AddRectFilled(innerMin, innerMax, Theme::toColor(Theme::withAlpha(Theme::ExitCellColor, 0.30f + 0.35f * pulse)), rounding);
                    drawList->AddRect(innerMin, innerMax, Theme::toColor(Theme::ExitCellColor), rounding, 0, 2.0f);
                    drawCellLabel(drawList, cellMin, cellSize, "E", Theme::ExitCellColor);
                    break;

                case CellType::Path:
                    if (visitedCells.at(row, column)) {
                        drawList->AddRectFilled(innerMin, innerMax, Theme::toColor(Theme::VisitedTrailColor), rounding);
                    }
                    drawCellLabel(drawList, cellMin, cellSize, to_string(cost), Theme::getCostColor(cost));
                    break;
            }
        }
    }

    // ---- player (glowing square) ----
    ImVec2 playerMin(origin.x + playerPosition.column * cellSize, origin.y + playerPosition.row * cellSize);
    ImVec2 playerMax(playerMin.x + cellSize, playerMin.y + cellSize);
    for (int layer = 3; layer >= 1; layer--) {
        float spread = layer * cellSize * 0.10f * (0.7f + 0.3f * pulse);
        drawList->AddRectFilled(ImVec2(playerMin.x - spread, playerMin.y - spread), ImVec2(playerMax.x + spread, playerMax.y + spread),
                                Theme::toColor(Theme::withAlpha(Theme::PlayerColor, 0.10f)), rounding * 2.0f);
    }
    drawList->AddRectFilled(ImVec2(playerMin.x + 2, playerMin.y + 2), ImVec2(playerMax.x - 2, playerMax.y - 2),
                            Theme::toColor(Theme::PlayerColor), rounding);
    drawList->AddCircleFilled(ImVec2(playerMin.x + cellSize * 0.5f, playerMin.y + cellSize * 0.5f), cellSize * 0.18f,
                              Theme::toColor(ImVec4(0.02f, 0.10f, 0.18f, 1.0f)));
}
