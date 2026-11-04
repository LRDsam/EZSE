#include "EZSE/Engine/Debug/DebugWidgets.h"

#include <imgui.h>

namespace ezse {

bool DragVec3(const char* label, Vec3& value, float speed) {
    float values[3] = {
        value.x,
        value.y,
        value.z
    };
    if (ImGui::DragFloat3(label, values, speed)) {
        value.x = values[0];
        value.y = values[1];
        value.z = values[2];
        return true;
    }
    return false;
}

bool ColorEditVec3(const char* label, Vec3& color) {
    float colors[3] = {
        color.x,
        color.y,
        color.z
    };
    if (ImGui::ColorEdit3(label, colors)) {
        color.x = colors[0];
        color.y = colors[1];
        color.z = colors[2];
        return true;
    }
    return false;
}

} // namespace ezse
