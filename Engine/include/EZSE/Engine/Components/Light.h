#pragma once

#include "EZSE/Math/Vec3.h"

#include <numbers>

namespace ezse {

// Directional: shines one way everywhere, like the sun (one is used, and it casts the shadows).
// Point: from one point in all directions. Spot: from one point in a cone. Ambient: reaches every
// surface from all sides, so nothing is fully black (one is used; it has no place or direction).
enum class LightType { Directional, Point, Spot, Ambient };

// A light; where it is and where it points come from the Transform of the same entity
// (it shines along its own -Z, like the camera looks). The Renderer gets color * intensity.
struct Light {
    LightType type = LightType::Directional;
    Vec3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f; // Point and Spot: how far the light reaches, in meters
    // Spot: the half angles from the axis, in radians. Full brightness inside innerAngle (30
    // degrees), no light outside outerAngle (45 degrees), a soft edge in between.
    float innerAngle = std::numbers::pi_v<float> / 6.0f;
    float outerAngle = std::numbers::pi_v<float> / 4.0f;
};

} // namespace ezse
