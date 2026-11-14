#pragma once
// =====================================================================================
//  PRESENTATION LAYER  -  Theme
//  ALL colours and the global ImGui style are here.   (Owner: Frontend developer)
//
//  IDEAS FOR THE TEAM:  load a custom font (see external/imgui/misc/fonts), try other
//  palettes (synthwave, toxic green, red alert), add rounded glass panels, animations.
// =====================================================================================
#include <imgui.h>
#include <algorithm>
using namespace std;

namespace Theme {

    // ---- background ----
    const ImVec4 BackgroundTop      (0.02f, 0.03f, 0.09f, 1.0f);
    const ImVec4 BackgroundBottom   (0.07f, 0.02f, 0.14f, 1.0f);
    const ImVec4 BackgroundGridLine (0.00f, 0.80f, 1.00f, 0.05f);

    // ---- text ----
    const ImVec4 TextNormal         (0.85f, 0.92f, 1.00f, 1.0f);
    const ImVec4 TextDim            (0.50f, 0.58f, 0.72f, 1.0f);

    // ---- the colours requested in the task: every important element has its own ----
    const ImVec4 StartButtonColor   (0.10f, 0.85f, 0.45f, 1.0f);   // green
    const ImVec4 ReplayButtonColor  (0.20f, 0.50f, 1.00f, 1.0f);   // blue
    const ImVec4 NewMazeButtonColor (0.70f, 0.30f, 1.00f, 1.0f);   // purple
    const ImVec4 HighScoreColor     (1.00f, 0.80f, 0.15f, 1.0f);   // gold
    const ImVec4 CurrentScoreColor  (0.15f, 0.90f, 1.00f, 1.0f);   // cyan
    const ImVec4 DangerColor        (1.00f, 0.25f, 0.35f, 1.0f);   // red

    // ---- maze squares ----
    const ImVec4 WallColor          (0.16f, 0.12f, 0.34f, 1.0f);
    const ImVec4 WallEdgeColor      (0.40f, 0.30f, 0.80f, 1.0f);
    const ImVec4 PathColor          (0.04f, 0.06f, 0.12f, 1.0f);
    const ImVec4 VisitedTrailColor  (0.00f, 0.85f, 1.00f, 0.22f);
    const ImVec4 StartCellColor     (0.10f, 0.85f, 0.45f, 1.0f);
    const ImVec4 ExitCellColor      (1.00f, 0.20f, 0.70f, 1.0f);
    const ImVec4 DoorCellColor      (1.00f, 0.65f, 0.10f, 1.0f);
    const ImVec4 PlayerColor        (0.20f, 0.95f, 1.00f, 1.0f);
    const ImVec4 CardBackground     (0.04f, 0.06f, 0.14f, 0.88f);
    const ImVec4 CardBorder         (0.00f, 0.85f, 1.00f, 0.40f);

    // ---- helpers ----
    inline ImU32 toColor(const ImVec4& color) { return ImGui::ColorConvertFloat4ToU32(color); }

    inline ImVec4 withAlpha(const ImVec4& color, float alpha) {
        return ImVec4(color.x, color.y, color.z, alpha);
    }

    inline ImVec4 mixColors(const ImVec4& first, const ImVec4& second, float amount) {
        return ImVec4(first.x + (second.x - first.x) * amount,
                      first.y + (second.y - first.y) * amount,
                      first.z + (second.z - first.z) * amount,
                      first.w + (second.w - first.w) * amount);
    }

    // Cheap square (1) = green ... expensive square (9) = hot red.
    inline ImVec4 getCostColor(int cost) {
        float amount = (float)(cost - 1) / 8.0f;
        amount = max(0.0f, min(1.0f, amount));
        return mixColors(ImVec4(0.25f, 1.00f, 0.55f, 1.0f), ImVec4(1.00f, 0.25f, 0.35f, 1.0f), amount);
    }

    // Global ImGui look. Called once after the ImGui context is created.
    inline void applyCyberStyle() {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding    = 0.0f;
        style.ChildRounding     = 10.0f;
        style.FrameRounding     = 8.0f;
        style.GrabRounding      = 8.0f;
        style.ScrollbarRounding = 10.0f;
        style.ScrollbarSize     = 20.0f;      // wide, easy to grab scroll bar
        style.ItemSpacing       = ImVec2(10, 10);
        style.WindowBorderSize  = 0.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text]                 = TextNormal;
        colors[ImGuiCol_TextDisabled]         = TextDim;
        colors[ImGuiCol_ChildBg]              = CardBackground;
        colors[ImGuiCol_Border]               = CardBorder;
        colors[ImGuiCol_FrameBg]              = ImVec4(0.06f, 0.09f, 0.20f, 1.0f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.09f, 0.14f, 0.30f, 1.0f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.10f, 0.18f, 0.36f, 1.0f);
        colors[ImGuiCol_SliderGrab]           = CurrentScoreColor;
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.60f, 1.00f, 1.00f, 1.0f);
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.03f, 0.04f, 0.10f, 0.80f);
        colors[ImGuiCol_ScrollbarGrab]        = withAlpha(CurrentScoreColor, 0.55f);
        colors[ImGuiCol_ScrollbarGrabHovered] = withAlpha(CurrentScoreColor, 0.80f);
        colors[ImGuiCol_ScrollbarGrabActive]  = CurrentScoreColor;
    }
}
