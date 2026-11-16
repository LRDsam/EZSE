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

Mat4 Rotation(const Quat& quat) {
    // The usual formula is written per row; Mat4 is m[column][row], so each column below is one
    // turned axis: column 0 is where the X axis ends up, column 1 the Y axis, column 2 the Z axis.
    const float xx = quat.x * quat.x;
    const float yy = quat.y * quat.y;
    const float zz = quat.z * quat.z;
    const float xy = quat.x * quat.y;
    const float xz = quat.x * quat.z;
    const float yz = quat.y * quat.z;
    const float wx = quat.w * quat.x;
    const float wy = quat.w * quat.y;
    const float wz = quat.w * quat.z;

    Mat4 mat;
    mat.m[0][0] = 1.0f - 2.0f * (yy + zz);
    mat.m[0][1] = 2.0f * (xy + wz);
    mat.m[0][2] = 2.0f * (xz - wy);

    mat.m[1][0] = 2.0f * (xy - wz);
    mat.m[1][1] = 1.0f - 2.0f * (xx + zz);
    mat.m[1][2] = 2.0f * (yz + wx);

    mat.m[2][0] = 2.0f * (xz + wy);
    mat.m[2][1] = 2.0f * (yz - wx);
    mat.m[2][2] = 1.0f - 2.0f * (xx + yy);
    return mat;
}

Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    const Vec3 f = Normalize(target - eye);
    const Vec3 r = Normalize(Cross(f, up));
    const Vec3 u = Normalize(Cross(r, f));

    // The camera axes are the rows: m[column][row].
    Mat4 view;
    view.m[0][0] = r.x;  view.m[1][0] = r.y;  view.m[2][0] = r.z;
    view.m[0][1] = u.x;  view.m[1][1] = u.y;  view.m[2][1] = u.z;
    view.m[0][2] = -f.x; view.m[1][2] = -f.y; view.m[2][2] = -f.z;

    // Move eye to the origin first: the rotation of -eye.
    view.m[3][0] = -Dot(r, eye);
    view.m[3][1] = -Dot(u, eye);
    view.m[3][2] = Dot(f, eye);
    return view;
}

Mat4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    // Each axis is scaled and shifted on its own, so the box maps onto the cube from -1 to 1.
    Mat4 projection;
    projection.m[0][0] = 2.0f / (right - left);
    projection.m[1][1] = 2.0f / (top - bottom);
    projection.m[2][2] = -2.0f / (farPlane - nearPlane);

    projection.m[3][0] = -(right + left) / (right - left);
    projection.m[3][1] = -(top + bottom) / (top - bottom);
    projection.m[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    return projection;
}

Mat4 Perspective(float fovY, float aspect, float nearPlane, float farPlane) {
    const float tangent = std::tan(fovY / 2.0f);

    Mat4 mat;
    mat.m[0][0] = 1.0f / (aspect * tangent);
    mat.m[1][1] = 1.0f / tangent;
    mat.m[2][2] = -((farPlane + nearPlane) / (farPlane - nearPlane));
    mat.m[3][2] = -((2.0f * farPlane * nearPlane) / (farPlane - nearPlane));
    mat.m[2][3] = -1.0f;
    mat.m[3][3] = 0.0f;
    return mat;
}

Vec3 TransformDirection(const Mat4& mat, const Vec3& direction) {
    const Vec4 result = mat * Vec4{ direction.x, direction.y, direction.z, 0.0f };
    return Vec3{ result.x, result.y, result.z };
}

} // namespace ezse
