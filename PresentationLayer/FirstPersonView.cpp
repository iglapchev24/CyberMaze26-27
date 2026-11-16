#include "FirstPersonView.h"
#include "Theme.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr float FOV = PI / 2.8f; // ~64 degrees
float clamp01(float value) { return std::max(0.0f, std::min(value, 1.0f)); }
ImU32 rgba(float r, float g, float b, float alpha = 1.0f) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(clamp01(r), clamp01(g), clamp01(b), clamp01(alpha)));
}

// Visibility test for a single floor tile. Never show prices or exit glow through walls.
// Sample the segment through the maze grid, stopping just short of the destination centre.
bool visibleFrom(const MazeData& maze, float x0, float y0, float x1, float y1) {
    const float dx = x1 - x0, dy = y1 - y0;
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length < 0.001f) return true;
    const int samples = std::max(1, static_cast<int>(std::ceil(length * 18.0f)));
    for (int i = 1; i < samples; ++i) {
        const float t = static_cast<float>(i) / samples;
        const int col = static_cast<int>(std::floor(x0 + dx * t));
        const int row = static_cast<int>(std::floor(y0 + dy * t));
        if (col < 0 || row < 0 || col >= maze.size || row >= maze.size ||
            maze.cellTypes.at(row, col) == CellType::Wall) return false;
    }
    return true;
}

struct ProjectedTile {
    int row, col;
    float depth;
    ImVec2 floorPoint;
};
}

void FirstPersonView::draw(const GameEngine& engine, ImDrawList* drawList,
                           ImVec2 origin, ImVec2 size, float viewAngle) {
    if (size.x < 20.0f || size.y < 20.0f) return;
    const MazeData& maze = engine.getMaze();
    if (maze.size <= 0) return;
    const GridPosition player = engine.getPlayerPosition();
    const float posX = static_cast<float>(player.column) + 0.5f;
    const float posY = static_cast<float>(player.row) + 0.5f;
    const float horizon = origin.y + size.y * 0.50f;
    const ImVec2 end(origin.x + size.x, origin.y + size.y);
    drawList->PushClipRect(origin, end, true);

    // Sky, floor, perspective floor lines, and horizon glow.
    drawList->AddRectFilledMultiColor(origin, ImVec2(end.x, horizon),
        rgba(0.018f, 0.025f, 0.075f), rgba(0.018f, 0.025f, 0.075f),
        rgba(0.075f, 0.085f, 0.18f), rgba(0.075f, 0.085f, 0.18f));
    drawList->AddRectFilledMultiColor(ImVec2(origin.x, horizon), end,
        rgba(0.07f, 0.065f, 0.15f), rgba(0.07f, 0.065f, 0.15f),
        rgba(0.012f, 0.018f, 0.045f), rgba(0.012f, 0.018f, 0.045f));
    for (int i = 1; i <= 16; ++i) {
        float t = static_cast<float>(i) / 17.0f;
        float y = horizon + size.y * 0.5f * (1.0f - 1.0f / (1.0f + t * 13.0f));
        drawList->AddLine(ImVec2(origin.x, y), ImVec2(end.x, y), rgba(0.0f, 0.58f, 0.75f, 0.13f));
    }

    // Classic grid DDA raycaster. Walls remain solid; doors are walkable as in GameEngine.
    const float dirX = std::cos(viewAngle);
    const float dirY = std::sin(viewAngle);
    const float planeScale = std::tan(FOV * 0.5f);
    const float planeX = -dirY * planeScale;
    const float planeY = dirX * planeScale;
    const int stripeWidth = 3;
    const int columns = std::max(1, static_cast<int>(std::ceil(size.x / stripeWidth)));
    for (int stripe = 0; stripe < columns; ++stripe) {
        const float cameraX = 2.0f * (stripe + 0.5f) / columns - 1.0f;
        const float rayX = dirX + planeX * cameraX;
        const float rayY = dirY + planeY * cameraX;
        int cellX = static_cast<int>(std::floor(posX));
        int cellY = static_cast<int>(std::floor(posY));
        const float deltaX = std::abs(rayX) < 1e-6f ? 1e30f : std::abs(1.0f / rayX);
        const float deltaY = std::abs(rayY) < 1e-6f ? 1e30f : std::abs(1.0f / rayY);
        const int stepX = rayX < 0 ? -1 : 1;
        const int stepY = rayY < 0 ? -1 : 1;
        float sideX = rayX < 0 ? (posX - cellX) * deltaX : (cellX + 1.0f - posX) * deltaX;
        float sideY = rayY < 0 ? (posY - cellY) * deltaY : (cellY + 1.0f - posY) * deltaY;
        bool sideHitY = false;
        float distance = 1.0f;
        int iteration = 0;
        for (; iteration < maze.size * 3 + 8; ++iteration) {
            if (sideX < sideY) {
                distance = sideX; sideX += deltaX; cellX += stepX; sideHitY = false;
            } else {
                distance = sideY; sideY += deltaY; cellY += stepY; sideHitY = true;
            }
            if (cellX < 0 || cellY < 0 || cellX >= maze.size || cellY >= maze.size ||
                maze.cellTypes.at(cellY, cellX) == CellType::Wall) break;
        }
        const float safeDist = std::max(0.08f, distance);
        const float wallHeight = std::min(size.y * 6.0f, size.y / safeDist);
        const float top = horizon - wallHeight * 0.5f;
        const float bottom = horizon + wallHeight * 0.5f;
        const float fog = std::max(0.13f, 1.0f - safeDist / (maze.size * 0.60f + 2.0f));
        const float shade = sideHitY ? 0.72f : 1.0f;
        const float x1 = origin.x + size.x * stripe / columns;
        const float x2 = origin.x + size.x * (stripe + 1) / columns + 0.5f;
        drawList->AddRectFilled(ImVec2(x1, top), ImVec2(x2, bottom),
            rgba(0.16f * fog * shade, 0.32f * fog * shade, 0.68f * fog * shade));
        // Neon-lit wall border strips communicate grid geometry and depth.
        if ((sideHitY ? cellX : cellY) % 2 == 0) {
            drawList->AddLine(ImVec2(x1, top), ImVec2(x2, top), rgba(0.12f * fog, 0.84f * fog, 1.0f * fog, 0.95f), 1.3f);
        }
        // Simple procedural masonry pattern near camera.
        if (wallHeight > 65.0f && safeDist < 10.0f) {
            for (int line = 1; line < 4; ++line) {
                const float y = top + wallHeight * line * 0.25f;
                if (y > origin.y && y < end.y)
                    drawList->AddLine(ImVec2(x1, y), ImVec2(x2, y), rgba(0.01f, 0.04f, 0.13f, 0.50f));
            }
        }
    }

    // Display each tile's ACTUAL scoring value on the floor, not a made-up 3D value.
    // The same GameEngine and maze.cellCosts remain authoritative in both views.
    // Tiles beyond a wall are excluded even if their screen projections overlap.
    std::vector<ProjectedTile> tiles;
    const float sideX = -dirY, sideY = dirX;
    const float cameraScale = size.x * 0.5f / planeScale;
    for (int row = 0; row < maze.size; ++row) {
        for (int col = 0; col < maze.size; ++col) {
            const CellType type = maze.cellTypes.at(row, col);
            if (type == CellType::Wall || type == CellType::Start) continue;
            const float dx = col + 0.5f - posX;
            const float dy = row + 0.5f - posY;
            const float forward = dx * dirX + dy * dirY;
            const float lateral = dx * sideX + dy * sideY;
            if (forward < 0.35f || forward > 14.0f ||
                std::abs(lateral) > forward * planeScale * 1.08f) continue;
            if (!visibleFrom(maze, posX, posY, col + 0.5f, row + 0.5f)) continue;
            // Center of tile projected to the floor below the horizon.
            const float x = origin.x + size.x * 0.5f + lateral * cameraScale / forward;
            const float y = horizon + size.y * 0.50f / forward;
            if (y < horizon + 8.0f || y > end.y - 8.0f) continue;
            tiles.push_back({row, col, forward, ImVec2(x, y)});
        }
    }
    // Far to near; nearby values remain readable when projected labels overlap.
    std::sort(tiles.begin(), tiles.end(), [](const ProjectedTile& a, const ProjectedTile& b) {
        return a.depth > b.depth;
    });
    for (const ProjectedTile& tile : tiles) {
        const CellType type = maze.cellTypes.at(tile.row, tile.col);
        // Exit gets a different marker; doors still show their actual cost.
        if (type == CellType::Exit) continue;
        const int cost = maze.cellCosts.at(tile.row, tile.col);
        const std::string label = std::to_string(cost);
        const float fontSize = std::max(13.0f, std::min(45.0f, size.y * 0.16f / tile.depth));
        const float tileSize = std::min(64.0f, size.y * 0.48f / tile.depth);
        const ImVec2 base = ImGui::CalcTextSize(label.c_str());
        const float factor = fontSize / ImGui::GetFontSize();
        const ImVec2 textPos(tile.floorPoint.x - base.x * factor * 0.5f,
                             tile.floorPoint.y - base.y * factor * 0.5f);
        const ImU32 color = Theme::toColor(type == CellType::Door
            ? Theme::DoorCellColor : Theme::getCostColor(cost));
        drawList->AddRectFilled(ImVec2(tile.floorPoint.x - tileSize * 0.48f, tile.floorPoint.y - tileSize * 0.36f),
            ImVec2(tile.floorPoint.x + tileSize * 0.48f, tile.floorPoint.y + tileSize * 0.36f),
            rgba(0.015f, 0.025f, 0.07f, 0.86f), 6.0f);
        drawList->AddRect(ImVec2(tile.floorPoint.x - tileSize * 0.48f, tile.floorPoint.y - tileSize * 0.36f),
            ImVec2(tile.floorPoint.x + tileSize * 0.48f, tile.floorPoint.y + tileSize * 0.36f),
            rgba(0.15f, 0.62f, 0.74f, 0.45f), 6.0f);
        drawList->AddText(ImGui::GetFont(), fontSize, ImVec2(textPos.x + 1.0f, textPos.y + 2.0f),
            rgba(0.0f, 0.0f, 0.0f, 0.95f), label.c_str());
        drawList->AddText(ImGui::GetFont(), fontSize, textPos, color, label.c_str());
    }

    // Pulsing beacon AT the actual exit. Only draw it if the exit is visible:
    // no wall-penetrating compass and no automatic solution to the maze puzzle.
    for (int row = 0; row < maze.size; ++row) {
        for (int col = 0; col < maze.size; ++col) {
            if (maze.cellTypes.at(row, col) != CellType::Exit) continue;
            const float dx = col + 0.5f - posX;
            const float dy = row + 0.5f - posY;
            const float depth = dx * dirX + dy * dirY;
            const float lateral = dx * sideX + dy * sideY;
            if (depth < 0.25f || depth > 20.0f ||
                std::abs(lateral) > depth * planeScale * 0.98f ||
                !visibleFrom(maze, posX, posY, col + 0.5f, row + 0.5f)) continue;
            const float x = origin.x + size.x * 0.5f + lateral * cameraScale / depth;
            const float baseY = horizon + size.y * 0.46f / depth;
            const float topY = horizon - size.y * 0.34f / depth;
            const float t = static_cast<float>(ImGui::GetTime());
            const float pulse = 0.8f + 0.2f * std::sin(t * 3.7f);
            const float radius = std::min(54.0f, std::max(5.0f, 40.0f / depth)) * pulse;
            const ImVec2 center(x, (topY + baseY) * 0.5f);
            drawList->AddLine(ImVec2(x, baseY), ImVec2(x, topY), rgba(1.0f, 0.35f, 0.7f, 0.7f), std::max(2.0f, radius * 0.35f));
            drawList->AddCircleFilled(center, radius * 2.5f, rgba(1.0f, 0.1f, 0.65f, 0.10f), 32);
            drawList->AddCircleFilled(center, radius * 1.7f, rgba(1.0f, 0.25f, 0.72f, 0.20f), 32);
            drawList->AddCircleFilled(center, radius, rgba(1.0f, 0.38f, 0.85f, 0.88f), 32);
            drawList->AddCircleFilled(center, radius * 0.40f, rgba(1.0f, 0.96f, 0.98f), 24);
            const char* text = "EXIT";
            const ImVec2 textSize = ImGui::CalcTextSize(text);
            if (radius >= 9.0f) {
                drawList->AddText(ImVec2(x - textSize.x * 0.5f, topY - 20.0f),
                    rgba(1.0f, 0.58f, 0.9f), text);
            }
        }
    }

    // Small cost legend. Green = cheaper, red = more expensive.
    drawList->AddText(ImVec2(origin.x + 18.0f, end.y - 27.0f),
        rgba(0.65f, 0.82f, 0.95f), "FLOOR NUMBERS = MOVE COST     GREEN: LOW     RED: HIGH");

    // Crosshair and small bottom status overlay.
    ImVec2 center(origin.x + size.x * 0.5f, horizon);
    drawList->AddLine(ImVec2(center.x - 11, center.y), ImVec2(center.x - 3, center.y), rgba(0.1f, 1.0f, 0.9f));
    drawList->AddLine(ImVec2(center.x + 3, center.y), ImVec2(center.x + 11, center.y), rgba(0.1f, 1.0f, 0.9f));
    drawList->AddLine(ImVec2(center.x, center.y - 11), ImVec2(center.x, center.y - 3), rgba(0.1f, 1.0f, 0.9f));
    drawList->AddLine(ImVec2(center.x, center.y + 3), ImVec2(center.x, center.y + 11), rgba(0.1f, 1.0f, 0.9f));
    drawList->AddRect(origin, end, Theme::toColor(Theme::CardBorder), 9.0f, 0, 2.0f);
    drawList->PopClipRect();
}
