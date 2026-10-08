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

} // namespace ezse
