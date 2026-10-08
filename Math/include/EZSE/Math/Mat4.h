#pragma once

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

} // namespace ezse
