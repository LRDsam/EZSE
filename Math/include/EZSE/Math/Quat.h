#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// A rotation by an angle around an axis of length 1: (x, y, z) = axis * sin(angle / 2) and
// w = cos(angle / 2). Only a quaternion of length 1 is a rotation. q and -q are the same
// orientation (one turns the other way around, by 360 degrees minus the angle).
// The default (0, 0, 0, 1) does not turn. The order x, y, z, w is also the one of glTF.
struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

// Part by part; used to mix quaternions in Slerp.
Quat operator+(const Quat& a, const Quat& b);
Quat operator-(const Quat& a, const Quat& b);
Quat operator-(const Quat& a);
Quat operator*(const Quat& a, float scalar);

// The Hamilton product: the rotation b followed by the rotation a, the same order as with Mat4.
// a * b is usually not b * a.
Quat operator*(const Quat& a, const Quat& b);

// a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w. For two rotations, the cosine of half the angle
// between them; negative when b is on the far side (then -b is the short way).
float Dot(const Quat& a, const Quat& b);

// The length: sqrt(x*x + y*y + z*z + w*w).
float Length(const Quat& q);

// The same rotation with length 1, to undo rounding after many multiplications.
// A quaternion that is (nearly) zero is no rotation; then the default (no turn) is returned.
Quat Normalize(const Quat& q);

// (-x, -y, -z, w): for a quaternion of length 1, the rotation that turns it back.
Quat Conjugate(const Quat& q);

// A turn by angle (radians) around axis. The axis must have length 1; it is not normalized here.
Quat AxisAngle(const Vec3& axis, float angle);

// Turns from a (t = 0) to b (t = 1) along the shortest arc, at a constant speed.
// a and b must have length 1; the result has length 1 as well.
Quat Slerp(const Quat& a, const Quat& b, float t);

} // namespace ezse
