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

} // namespace ezse
