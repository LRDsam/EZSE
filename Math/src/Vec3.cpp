#include "EZSE/Math/Vec3.h"

namespace ezse {

    Vec3 operator+(const Vec3& a, const Vec3& b) {
        return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
    }

    Vec3 operator-(const Vec3& a) {
        return Vec3{ -a.x, -a.y, -a.z };
    }


} // namespace ezse
