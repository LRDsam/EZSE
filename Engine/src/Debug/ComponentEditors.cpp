#include "EZSE/Engine/Debug/ComponentEditors.h"

#include "EZSE/Engine/Debug/DebugWidgets.h"

#include "EZSE/Math/Quat.h"

#include <imgui.h>
#include <algorithm>

namespace ezse {

bool EditTransform(Transform& transform) {
    bool changed = false;
    changed |= DragVec3("Position", transform.position, 0.05f);

    // The sliders show angles, computed again every frame. Only a change of an angle writes back,
    // so moving the position or the scale does not round the rotation through angles. Near a
    // pitch of +-90 degrees yaw and roll can jump in the sliders; the orientation stays correct.
    Vec3 angles = ToEuler(transform.rotation);
    bool rotationChanged = false;
    rotationChanged |= ImGui::SliderAngle("Pitch", &angles.x);
    rotationChanged |= ImGui::SliderAngle("Yaw", &angles.y);
    rotationChanged |= ImGui::SliderAngle("Roll", &angles.z);
    if (rotationChanged) {
        transform.rotation = FromEuler(angles);
    }

    changed |= DragVec3("Scale", transform.scale, 0.05f);
    return changed || rotationChanged;
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
