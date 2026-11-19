#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// A light at one point that shines in all directions and fades out towards range (meters). The
// color is already multiplied by the intensity.
struct PointLight {
    Vec3 position = Vec3{};
    Vec3 color = {1.0f, 1.0f, 1.0f};
    float range = 10.0f;
};

} // namespace ezse
