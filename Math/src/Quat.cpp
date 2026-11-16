#include "EZSE/Math/Quat.h"

#include <cmath>

namespace ezse {

Quat operator+(const Quat& a, const Quat& b) {
    return Quat{
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        a.w + b.w
    };
}

Quat operator-(const Quat& a, const Quat& b) {
    return Quat{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
        a.w - b.w
    };
}

Quat operator-(const Quat& a) {
    return Quat{-a.x, -a.y, -a.z, -a.w};
}

Quat operator*(const Quat& a, const Quat& b) {
    return Quat{
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

Quat operator*(const Quat& a, float scalar) {
    return Quat{
        a.x * scalar,
        a.y * scalar,
        a.z * scalar,
        a.w * scalar
    };
}

float Dot(const Quat& a, const Quat& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float Length(const Quat& q) {
    return std::sqrt(Dot(q, q));
}

Quat Normalize(const Quat& q) {
    const float margin = 1e-6f;
    const float length = Length(q);
    if (length < margin) {
        return Quat{};
    }
    // One division and four multiplications instead of four divisions.
    return q * (1.0f / length);
}

Quat Conjugate(const Quat& q) {
    return Quat{-q.x, -q.y, -q.z, q.w};
}

Quat AxisAngle(const Vec3& axis, float angle) {
    const float halfAngle = angle * 0.5f;
    const float sinHalfAngle = std::sin(halfAngle);
    return Quat{
        axis.x * sinHalfAngle,
        axis.y * sinHalfAngle,
        axis.z * sinHalfAngle,
        std::cos(halfAngle)
    };
}

Quat Slerp(const Quat& a, const Quat& b, float t) {
    // Above this the two are almost the same orientation: sin(angle) is then close to zero, so
    // mixing part by part and normalizing is both safe and accurate enough. It also keeps acos
    // away from values just above 1 that rounding can produce.
    constexpr float c_nearlyEqual = 0.9995f;

    // q and -q are the same orientation; with a negative dot, -b is the short way.
    float dot = Dot(a, b);
    Quat end = b;
    if (dot < 0.0f) {
        dot = -dot;
        end = -b;
    }
    if (dot > c_nearlyEqual) {
        return Normalize(a + (end - a) * t);
    }

    const float angle = std::acos(dot);
    const float sinAngle = std::sin(angle);
    return a * (std::sin((1.0f - t) * angle) / sinAngle) +
           end * (std::sin(t * angle) / sinAngle);
}

Quat FromEuler(const Vec3& angles) {
    // a * b is first b, then a: roll (Z) on the right is applied first, yaw (Y) on the left last.
    return AxisAngle(Vec3{0.0f, 1.0f, 0.0f}, angles.y) *
           AxisAngle(Vec3{1.0f, 0.0f, 0.0f}, angles.x) *
           AxisAngle(Vec3{0.0f, 0.0f, 1.0f}, angles.z);
}

Vec3 ToEuler(const Quat& q) {
    // The angles are read from elements of the rotation matrix of q, written per row as R[row][col],
    // for R = RotationY(yaw) * RotationX(pitch) * RotationZ(roll):
    //   R[1][2] = -sin(pitch)
    //   R[0][2] = sin(yaw) cos(pitch),   R[2][2] = cos(yaw) cos(pitch)
    //   R[1][0] = cos(pitch) sin(roll),  R[1][1] = cos(pitch) cos(roll)
    // Each element is computed straight from the quaternion, without building the matrix.

    // Below this, cos(pitch) is so small (pitch within 0.006 degrees of +-90) that yaw and roll
    // can no longer be told apart.
    constexpr float c_gimbalLock = 1e-4f;

    const float sinPitch = -2.0f * (q.y * q.z - q.w * q.x);
    const float r10 = 2.0f * (q.x * q.y + q.w * q.z);
    const float r11 = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    // cos(pitch) from the length of (R[1][0], R[1][1]). atan2 instead of asin(sinPitch): asin is
    // very inaccurate near +-1 (a rounding error of 1e-7 gives 3e-4 radians) and gives NaN when
    // rounding pushes the value just above 1.
    const float cosPitch = std::sqrt(r10 * r10 + r11 * r11);
    const float pitch = std::atan2(sinPitch, cosPitch);

    if (cosPitch < c_gimbalLock) {
        // Gimbal lock: roll is 0 and the whole turn goes into yaw, read from
        // R[2][0] = -sin(yaw) and R[0][0] = cos(yaw), which hold when roll is 0.
        const float r20 = 2.0f * (q.x * q.z - q.w * q.y);
        const float r00 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
        return Vec3{pitch, std::atan2(-r20, r00), 0.0f};
    }

    const float r02 = 2.0f * (q.x * q.z + q.w * q.y);
    const float r22 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    return Vec3{pitch, std::atan2(r02, r22), std::atan2(r10, r11)};
}

} // namespace ezse
