#pragma once

namespace ezse {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a);
Vec3 operator*(const Vec3& vec, float scalar);

// The length of the vector: sqrt(x*x + y*y + z*z).
float Length(const Vec3& vec);

// A vector in the same direction with length 1.
// A vector that is (nearly) zero has no direction; then the zero vector is returned.
Vec3 Normalize(const Vec3& vec);

} // namespace ezse
