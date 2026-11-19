#pragma once

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Quat.h"
#include "EZSE/Math/Vec3.h"

namespace ezse {

// Building blocks for a model matrix. A model matrix is usually
// Translation(position) * Rotation(rotation) * Scale(scale): scale, then turn, then move.

// Moves a point by offset; a direction stays the same.
Mat4 Translation(const Vec3& offset);
// Stretches along the axes: factor 2 doubles that axis, 1 leaves it.
Mat4 Scale(const Vec3& factor);
// A turn by angle (radians) around one axis. A positive angle turns counterclockwise when you
// look from the positive end of the axis towards the origin: RotationY(+90 degrees) turns -Z
// (forward) to -X (left).
Mat4 RotationX(float angle);
Mat4 RotationY(float angle);
Mat4 RotationZ(float angle);

// The rotation matrix of a quaternion of length 1; Rotation(AxisAngle(Y, a)) equals RotationY(a).
Mat4 Rotation(const Quat& quat);

// A view matrix for a camera at eye that looks at target. up is roughly "up" for the camera
// (usually {0, 1, 0}) and must not be parallel to target - eye.
Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

// A projection without perspective: the box from left to right, bottom to top and nearPlane to
// farPlane in front of the camera (along -Z) becomes the cube from -1 to 1.
Mat4 Orthographic(float left, float right, float bottom, float top, float nearPlane,
                  float farPlane);

// A projection with perspective for a camera that looks along -Z: what is further away is
// smaller. fovY is the vertical field of view in radians, aspect is width / height, and nearPlane
// and farPlane (both larger than 0) are the distances between which anything is drawn. Depth ends
// up from -1 (near) to 1 (far), as OpenGL expects.
Mat4 Perspective(float fovY, float aspect, float nearPlane, float farPlane);

// Turns a direction with the rotation (and scale) of the matrix; the translation is left out.
Vec3 TransformDirection(const Mat4& mat, const Vec3& direction);

} // namespace ezse
