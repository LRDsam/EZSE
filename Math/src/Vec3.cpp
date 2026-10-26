#include "EZSE/Math/Vec3.h"

#include <cmath>

namespace ezse {

Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 operator-(const Vec3& a) {
    return Vec3{ -a.x, -a.y, -a.z };
}

Vec3 operator*(const Vec3& vec, float scalar) {
    return Vec3{ vec.x * scalar, vec.y * scalar, vec.z * scalar };
}

float Length(const Vec3& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

Vec3 Normalize(const Vec3& vec) {
    const float margin = 1e-6f;
    const float length = Length(vec);
    if (length < margin) {
        return Vec3{};
    }
    return Vec3{ vec.x / length, vec.y / length, vec.z / length };
}

} // namespace ezse
