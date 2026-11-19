#pragma once

#include "EZSE/Math/Vec4.h"

namespace ezse {

// A 4x4 matrix, stored column by column as OpenGL expects: m[column][row]. Column 3 holds the
// translation. The default is the identity, which changes nothing.
struct Mat4 {
    float m[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
};

// a * b applies b first and then a: Translation(t) * Rotation(r) turns first, then moves.
Mat4 operator*(const Mat4& a, const Mat4& b);
// The vector transformed by the matrix. w = 1 for a point (moved by the translation), w = 0 for a
// direction (not moved).
Vec4 operator*(const Mat4& mat, const Vec4& vec);

// The matrix that undoes mat: mat * Inverse(mat) is the identity. Works for any matrix, also a
// projection. Returns the identity when mat cannot be inverted (its determinant is almost 0, for
// example with a scale of 0).
Mat4 Inverse(const Mat4& mat);

} // namespace ezse
