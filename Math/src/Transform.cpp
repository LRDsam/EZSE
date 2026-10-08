#include "EZSE/Math/Transform.h"

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

} // namespace ezse
