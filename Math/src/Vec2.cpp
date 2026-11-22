#include "EZSE/Math/Vec2.h"

#include <cmath>

namespace ezse {

Vec2 operator+(const Vec2& a, const Vec2& b) {
    return Vec2{a.x + b.x, a.y + b.y};
}

Vec2 operator-(const Vec2& a, const Vec2& b) {
    return Vec2{a.x - b.x, a.y - b.y};
}

Vec2 operator-(const Vec2& a) {
    return Vec2{-a.x, -a.y};
}

Vec2 operator*(const Vec2& vec, float scalar) {
    return Vec2{vec.x * scalar, vec.y * scalar};
}

float Dot(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
}

float Length(const Vec2& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

Vec2 Normalize(const Vec2& vec) {
    const float margin = 1e-6f;
    const float length = Length(vec);
    if (length < margin) {
        return Vec2{};
    }
    return Vec2{vec.x / length, vec.y / length};
}

Vec2 Lerp(const Vec2& a, const Vec2& b, float t) {
    return a + (b - a) * t;
}

} // namespace ezse
