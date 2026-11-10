#pragma once

#include "EZSE/Engine/Components/Light.h"
#include "EZSE/Engine/Components/Transform.h"

namespace ezse {

// ImGui editors for built-in components. Each returns true in the frame something changed.
// They use plain labels; call ImGui::PushID/PopID around each entity so two editors don't clash.

// Position, rotation as pitch, yaw and roll in degrees, and scale.
bool EditTransform(Transform& transform);
// Only the fields that matter for light.type; the type itself cannot be changed here.
bool EditLight(Light& light);

} // namespace ezse
