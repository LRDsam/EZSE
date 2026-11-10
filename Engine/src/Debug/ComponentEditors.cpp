#include "EZSE/Engine/Debug/ComponentEditors.h"

#include "EZSE/Engine/Debug/DebugWidgets.h"

#include <imgui.h>
#include <algorithm>

namespace ezse {

bool EditTransform(Transform& transform) {
    bool changed = false;
    changed |= DragVec3("Position", transform.position, 0.05f);
    changed |= ImGui::SliderAngle("Pitch", &transform.rotation.x);
    changed |= ImGui::SliderAngle("Yaw", &transform.rotation.y);
    changed |= ImGui::SliderAngle("Roll", &transform.rotation.z);
    changed |= DragVec3("Scale", transform.scale, 0.05f);
    return changed;
}

bool EditLight(Light& light) {
    bool changed = false;
    changed |= ColorEditVec3("Color", light.color);
    changed |= ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 10.0f);
    if (light.type == LightType::Point || light.type == LightType::Spot) {
        changed |= ImGui::SliderFloat("Range", &light.range, 0.1f, 20.0f);
    }
    if (light.type == LightType::Spot) {
        changed |= ImGui::SliderAngle("Outer angle", &light.outerAngle, 1.0f, 89.0f);
        changed |= ImGui::SliderAngle("Inner angle", &light.innerAngle, 0.0f, 89.0f);
        light.innerAngle = std::min(light.innerAngle, light.outerAngle - 0.001f);
    }
    return changed;
}

} // namespace ezse
