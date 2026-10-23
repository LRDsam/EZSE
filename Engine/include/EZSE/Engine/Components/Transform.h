#pragma once

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"

namespace ezse {

struct Transform {
    Vec3 position;
    Vec3 rotation; // radians around X, Y and Z; applied as Z, then X, then Y
    Vec3 scale = { 1.0f, 1.0f, 1.0f };
};

Mat4 ModelMatrix(const Transform& transform);
Mat4 ViewMatrix(const Transform& transform);

} // namespace ezse
