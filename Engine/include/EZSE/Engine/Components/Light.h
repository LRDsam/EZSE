#pragma once

#include "EZSE/Math/Vec3.h"

#include <numbers>

namespace ezse {

enum class LightType { Directional, Point, Spot, Ambient };

// A light; where it is and where it points come from the Transform of the same entity
// (it shines along its own -Z, like the camera looks). The Renderer gets color * intensity.
struct Light {
    LightType type = LightType::Directional;
    Vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f; // Point and Spot: how far the light reaches, in meters
    float innerAngle = std::numbers::pi_v<float> / 6.0f; // Spot: full brightness inside, radians (30 degrees)
    float outerAngle = std::numbers::pi_v<float> / 4.0f; // Spot: no light outside, radians (45 degrees)
};

} // namespace ezse
