#pragma once

namespace ezse {

// A point or direction in a plane, such as the walking direction of a player (x sideways, y
// forward) or a position on the screen.
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

// Part by part; a vector times a number scales its length.
Vec2 operator+(const Vec2& a, const Vec2& b);
Vec2 operator-(const Vec2& a, const Vec2& b);
Vec2 operator-(const Vec2& a);
Vec2 operator*(const Vec2& vec, float scalar);

// The dot product: a.x * b.x + a.y * b.y, which is |a| * |b| * cos(angle).
float Dot(const Vec2& a, const Vec2& b);

// The length of the vector: sqrt(x*x + y*y).
float Length(const Vec2& vec);

// A vector in the same direction with length 1.
// A vector that is (nearly) zero has no direction; then the zero vector is returned.
Vec2 Normalize(const Vec2& vec);

// The point a fraction t of the way from a to b: a + (b - a) * t. t = 0 gives a, t = 1 gives b.
Vec2 Lerp(const Vec2& a, const Vec2& b, float t);

} // namespace ezse
