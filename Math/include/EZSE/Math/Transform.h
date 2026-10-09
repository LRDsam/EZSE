#pragma once

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"

namespace ezse {

Mat4 Translation(const Vec3& offset);
Mat4 Scale(const Vec3& factor);

Mat4 RotationX(float angle);
Mat4 RotationY(float angle);
Mat4 RotationZ(float angle);

} // namespace ezse
