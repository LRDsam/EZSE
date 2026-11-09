#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

struct PointLight {
    Vec3 position = Vec3{};
    Vec3 color = { 1.0f, 1.0f, 1.0f };
    float range = 10.0f;
};

} // namespace ezse
