#pragma once

namespace ezse {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

Vec3 operator+(const Vec3& a, const Vec3& b);

Vec3 operator-(const Vec3& a);


} // namespace ezse
