#pragma once

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"

namespace ezse {

Mat4 Translation(const Vec3& offset);
Mat4 Scale(const Vec3& factor);
Mat4 RotationX(float angle);
Mat4 RotationY(float angle);
Mat4 RotationZ(float angle);

// A view matrix for a camera at eye that looks at target. up is roughly "up" for the camera
// (usually {0, 1, 0}) and must not be parallel to target - eye.
Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

// A projection without perspective: the box from left to right, bottom to top and nearPlane to
// farPlane in front of the camera (along -Z) becomes the cube from -1 to 1.
Mat4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

Mat4 Perspective(float fovY, float aspect, float nearPlane, float farPlane);

// Turns a direction with the rotation (and scale) of the matrix; the translation is left out.
Vec3 TransformDirection(const Mat4& mat, const Vec3& direction);

} // namespace ezse
