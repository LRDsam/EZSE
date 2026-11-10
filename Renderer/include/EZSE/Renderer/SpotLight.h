#pragma once

#include "EZSE/Math/Vec3.h"

#include <numbers>

namespace ezse {

struct SpotLight {
    Vec3  position = Vec3{};
    Vec3  direction = {0.0f,0.0f,-1.0f};   // which way the cone points; need not be normalized
    Vec3  color = {1.0f,1.0f,1.0f}; // already multiplied by intensity
    float range = 10.0f;
    float innerAngle = std::numbers::pi_v<float> / 6.0f; // half angle from the axis, radians
    float outerAngle = std::numbers::pi_v<float> / 4.0f;
};

} // namespace ezse
