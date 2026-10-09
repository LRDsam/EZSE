#include "EZSE/Math/Mat4.h"

namespace ezse {
Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 answer;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            answer.m[col][row] = 0.0f;
            for (int i = 0; i < 4; ++i) {
                answer.m[col][row] += a.m[i][row] * b.m[col][i];
            }
        }
    }
    return answer;
}

Vec4 operator*(const Mat4& mat, const Vec4& vec) {
    return Vec4{
        (mat.m[0][0] * vec.x) + (mat.m[1][0] * vec.y) + (mat.m[2][0] * vec.z) + (mat.m[3][0] * vec.w),
        (mat.m[0][1] * vec.x) + (mat.m[1][1] * vec.y) + (mat.m[2][1] * vec.z) + (mat.m[3][1] * vec.w),
        (mat.m[0][2] * vec.x) + (mat.m[1][2] * vec.y) + (mat.m[2][2] * vec.z) + (mat.m[3][2] * vec.w),
        (mat.m[0][3] * vec.x) + (mat.m[1][3] * vec.y) + (mat.m[2][3] * vec.z) + (mat.m[3][3] * vec.w),
    };
}

} // namespace ezse
