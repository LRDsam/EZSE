#pragma once

namespace ezse {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a);
Vec3 operator*(const Vec3& vec, float scalar);

// A vector perpendicular to both a and b, with length |a| * |b| * sin(angle); zero when a and b
// are parallel. The direction follows the right-hand rule: Cross(X, Y) is Z, and
// Cross(b, a) is -Cross(a, b).
Vec3 Cross(const Vec3& a, const Vec3& b);

// The dot product: a.x * b.x + a.y * b.y + a.z * b.z, which is |a| * |b| * cos(angle).
// For two vectors of length 1 it is the cosine of the angle between them.
float Dot(const Vec3& a, const Vec3& b);

// The length of the vector: sqrt(x*x + y*y + z*z).
float Length(const Vec3& vec);

// A vector in the same direction with length 1.
// A vector that is (nearly) zero has no direction; then the zero vector is returned.
Vec3 Normalize(const Vec3& vec);

} // namespace ezse
