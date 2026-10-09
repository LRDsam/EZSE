#include "EZSE/Math/Transform.h"

#include <cmath>

namespace ezse {

Mat4 Translation(const Vec3& offset) {
    Mat4 mat;
    mat.m[3][0] = offset.x;
    mat.m[3][1] = offset.y;
    mat.m[3][2] = offset.z;
    return mat;
}

Mat4 Scale(const Vec3& factor) {
    Mat4 mat;
    mat.m[0][0] = factor.x;
    mat.m[1][1] = factor.y;
    mat.m[2][2] = factor.z;
    return mat;
}

Mat4 RotationX(float angle) {
    const float sine = std::sin(angle);
    const float cosine = std::cos(angle);

    Mat4 mat;
    mat.m[1][1] = cosine;
    mat.m[2][1] = -sine;
    mat.m[1][2] = sine;
    mat.m[2][2] = cosine;
    return mat;
}

Mat4 RotationY(float angle) {
    const float sine = std::sin(angle);
    const float cosine = std::cos(angle);

    Mat4 mat;
    mat.m[0][0] = cosine;
    mat.m[2][0] = sine;
    mat.m[0][2] = -sine;
    mat.m[2][2] = cosine;
    return mat;
}

Mat4 RotationZ(float angle) {
    const float sine = std::sin(angle);
    const float cosine = std::cos(angle);

    Mat4 mat;
    mat.m[0][0] = cosine;
    mat.m[1][0] = -sine;
    mat.m[0][1] = sine;
    mat.m[1][1] = cosine;
    return mat;
}

} // namespace ezse
