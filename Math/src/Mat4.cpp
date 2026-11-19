#include "EZSE/Math/Mat4.h"

#include <cmath>

namespace ezse {

namespace {

// The determinant of the 3x3 matrix that is left after removing one column and one row. Without
// the sign of the cofactor.
float Minor(const Mat4& mat, int skipColumn, int skipRow) {
    int columns[3];
    int rows[3];
    int columnCount = 0;
    int rowCount = 0;
    for (int i = 0; i < 4; ++i) {
        if (i != skipColumn) {
            columns[columnCount++] = i;
        }
        if (i != skipRow) {
            rows[rowCount++] = i;
        }
    }

    // The 3x3 as  | a b c |   with a column per letter column and a row per line.
    //             | d e f |
    //             | g h i |
    const float a = mat.m[columns[0]][rows[0]];
    const float b = mat.m[columns[1]][rows[0]];
    const float c = mat.m[columns[2]][rows[0]];
    const float d = mat.m[columns[0]][rows[1]];
    const float e = mat.m[columns[1]][rows[1]];
    const float f = mat.m[columns[2]][rows[1]];
    const float g = mat.m[columns[0]][rows[2]];
    const float h = mat.m[columns[1]][rows[2]];
    const float i = mat.m[columns[2]][rows[2]];

    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

} // namespace

Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 answer;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            answer.m[col][row] = 0.0f;
            for (int i = 0; i < 4; ++i) {
                answer.m[col][row] += a.m[i][row] * b.m[col][i];
            }
        }
    }
    return answer;
}

Vec4 operator*(const Mat4& mat, const Vec4& vec) {
    // Each row of the result: that row of the matrix times the vector.
    return Vec4{
        mat.m[0][0] * vec.x + mat.m[1][0] * vec.y + mat.m[2][0] * vec.z + mat.m[3][0] * vec.w,
        mat.m[0][1] * vec.x + mat.m[1][1] * vec.y + mat.m[2][1] * vec.z + mat.m[3][1] * vec.w,
        mat.m[0][2] * vec.x + mat.m[1][2] * vec.y + mat.m[2][2] * vec.z + mat.m[3][2] * vec.w,
        mat.m[0][3] * vec.x + mat.m[1][3] * vec.y + mat.m[2][3] * vec.z + mat.m[3][3] * vec.w,
    };
}

Mat4 Inverse(const Mat4& mat) {
    // The cofactors: every minor with the sign of a checkerboard, + where column + row is even.
    float cofactors[4][4];
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            const float sign = (column + row) % 2 == 0 ? 1.0f : -1.0f;
            cofactors[column][row] = sign * Minor(mat, column, row);
        }
    }

    // The determinant along row 0, from the cofactors that are needed anyway.
    float det = 0.0f;
    for (int column = 0; column < 4; ++column) {
        det += mat.m[column][0] * cofactors[column][0];
    }

    const float margin = 1e-6f;
    if (std::abs(det) < margin) {
        return Mat4{};
    }

    // The inverse is the adjugate (the cofactors transposed) divided by the determinant.
    const float invDet = 1.0f / det;
    Mat4 result;
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            result.m[row][column] = cofactors[column][row] * invDet;
        }
    }
    return result;
}

} // namespace ezse
