#include "UiWidgets.h"
#include "Theme.h"
#include <algorithm>
using namespace std;

namespace UiWidgets {

void drawScaledText(const string& text, float scale, const ImVec4& color, bool centered, float areaWidth, bool glow) {
    ImVec2 baseSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 scaledSize(baseSize.x * scale, baseSize.y * scale);
    float scaledFontSize = ImGui::GetFontSize() * scale;

    ImVec2 position = ImGui::GetCursorScreenPos();
    if (centered) {
        position.x += (areaWidth - scaledSize.x) * 0.5f;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (glow) {
        for (int layer = 3; layer >= 1; layer--) {
            float spread = layer * 1.5f;
            ImU32 haloColor = Theme::toColor(Theme::withAlpha(color, 0.10f));
            drawList->AddText(ImGui::GetFont(), scaledFontSize, ImVec2(position.x - spread, position.y), haloColor, text.c_str());
            drawList->AddText(ImGui::GetFont(), scaledFontSize, ImVec2(position.x + spread, position.y), haloColor, text.c_str());
            drawList->AddText(ImGui::GetFont(), scaledFontSize, ImVec2(position.x, position.y - spread), haloColor, text.c_str());
            drawList->AddText(ImGui::GetFont(), scaledFontSize, ImVec2(position.x, position.y + spread), haloColor, text.c_str());
        }
    }
    drawList->AddText(ImGui::GetFont(), scaledFontSize, position, Theme::toColor(color), text.c_str());

    ImGui::SetCursorScreenPos(position);
    ImGui::Dummy(scaledSize);
}

bool glowButton(const string& label, ImVec2 size, const ImVec4& baseColor, bool enabled) {
    ImGui::BeginDisabled(!enabled);
    ImGui::PushStyleColor(ImGuiCol_Button,        Theme::withAlpha(baseColor, 0.22f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::withAlpha(baseColor, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Theme::withAlpha(baseColor, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Border,        Theme::withAlpha(baseColor, 0.90f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);

    bool wasClicked = ImGui::Button(label.c_str(), size);

    if (ImGui::IsItemHovered() && enabled) {
        ImVec2 corner1 = ImGui::GetItemRectMin();
        ImVec2 corner2 = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRect(ImVec2(corner1.x - 3, corner1.y - 3), ImVec2(corner2.x + 3, corner2.y + 3),
                                            Theme::toColor(Theme::withAlpha(baseColor, 0.35f)), 10.0f, 0, 3.0f);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    ImGui::EndDisabled();
    return wasClicked && enabled;
}

bool toggleSwitch(const string& id, bool* value) {
    float height = ImGui::GetFrameHeight();
    float width = height * 1.9f;
    ImVec2 position = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(id.c_str(), ImVec2(width, height));
    bool wasChanged = false;
    if (ImGui::IsItemClicked()) {
        *value = !*value;
        wasChanged = true;
    }

    // Smooth animation: the animated position is stored per widget id.
    ImGuiStorage* storage = ImGui::GetStateStorage();
    ImGuiID widgetId = ImGui::GetItemID();
    float target = *value ? 1.0f : 0.0f;
    float animated = storage->GetFloat(widgetId, target);
    animated += (target - animated) * min(1.0f, ImGui::GetIO().DeltaTime * 14.0f);
    storage->SetFloat(widgetId, animated);

    ImVec4 trackColor = Theme::mixColors(ImVec4(0.15f, 0.18f, 0.30f, 1.0f), Theme::DangerColor, animated);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float radius = height * 0.5f;
    drawList->AddRectFilled(position, ImVec2(position.x + width, position.y + height), Theme::toColor(trackColor), radius);
    drawList->AddRect(position, ImVec2(position.x + width, position.y + height),
                      Theme::toColor(Theme::withAlpha(Theme::TextNormal, 0.35f)), radius, 0, 1.5f);
    float knobX = position.x + radius + animated * (width - 2.0f * radius);
    drawList->AddCircleFilled(ImVec2(knobX, position.y + radius), radius - 3.0f, Theme::toColor(Theme::TextNormal));
    return wasChanged;
}

void statChip(const string& caption, const string& valueText, const ImVec4& color, ImVec2 size) {
    ImVec2 position = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 bottomRight(position.x + size.x, position.y + size.y);

    drawList->AddRectFilled(position, bottomRight, Theme::toColor(Theme::withAlpha(color, 0.10f)), 10.0f);
    drawList->AddRect(position, bottomRight, Theme::toColor(Theme::withAlpha(color, 0.85f)), 10.0f, 0, 1.5f);
    drawList->AddText(ImVec2(position.x + 14, position.y + 8), Theme::toColor(Theme::withAlpha(color, 0.85f)), caption.c_str());

    float valueFontSize = ImGui::GetFontSize() * 2.1f;
    drawList->AddText(ImGui::GetFont(), valueFontSize, ImVec2(position.x + 14, position.y + 26), Theme::toColor(color), valueText.c_str());

    ImGui::Dummy(size);
}

void beginCard(const string& id, float width, const ImVec4& borderColor) {
    ImGui::PushStyleColor(ImGuiCol_Border, Theme::withAlpha(borderColor, 0.55f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18, 14));
    ImGui::BeginChild(id.c_str(), ImVec2(width, 0),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
}

void endCard() {
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

} // namespace UiWidgets
