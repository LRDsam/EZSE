#include "EZSE/Math/Vec3.h"

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

} // namespace ezse
