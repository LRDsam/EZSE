#pragma once

#include "EZSE/Math/Vec4.h"

namespace ezse {

//4x4 Matrix
struct Mat4 {
    float m[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
};

Mat4 operator*(const Mat4& a, const Mat4& b);
Vec4 operator*(const Mat4& mat, const Vec4& vec);

// The matrix that undoes mat: mat * Inverse(mat) is the identity. Works for any matrix, also a
// projection. Returns the identity when mat cannot be inverted (its determinant is almost 0, for
// example with a scale of 0).
Mat4 Inverse(const Mat4& mat);

} // namespace ezse
