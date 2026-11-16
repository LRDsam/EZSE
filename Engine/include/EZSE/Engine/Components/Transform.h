#pragma once

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Quat.h"
#include "EZSE/Math/Vec3.h"

namespace ezse {

struct Transform {
    Vec3 position;
    // The orientation, of length 1; the default does not turn. Make one from angles with
    // FromEuler (pitch, yaw, roll in radians).
    Quat rotation;
    Vec3 scale = { 1.0f, 1.0f, 1.0f };
};

Mat4 ModelMatrix(const Transform& transform);
Mat4 ViewMatrix(const Transform& transform);

} // namespace ezse
