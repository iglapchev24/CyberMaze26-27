#pragma once
// =====================================================================================
//  PRESENTATION LAYER  -  reusable UI widgets (text, glow buttons, toggle, score chips)
//  (Owner: Frontend developer)
// =====================================================================================
#include <imgui.h>
#include <string>
using namespace std;

namespace UiWidgets {

    // Big (or small) text drawn with the draw list. If centered, it is centred inside
    // areaWidth. glow = true draws a soft halo behind the letters.
    void drawScaledText(const string& text, float scale, const ImVec4& color, bool centered, float areaWidth, bool glow = false);

    // Button with a coloured neon outline and hover glow. Returns true when clicked.
    bool glowButton(const string& label, ImVec2 size, const ImVec4& baseColor, bool enabled = true);

    // Animated on/off switch. Returns true when the value was changed this frame.
    bool toggleSwitch(const string& id, bool* value);

    // Rounded box with a small caption and a big value (used for scores).
    void statChip(const string& caption, const string& valueText, const ImVec4& color, ImVec2 size);

    // Card = bordered panel that grows with its content. Always pair begin/end.
    void beginCard(const string& id, float width, const ImVec4& borderColor);
    void endCard();
}
