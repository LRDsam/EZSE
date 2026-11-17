#include "EZSE/Engine/Components/WorldMatrix.h"

#include "EZSE/Math/Transform.h"

namespace ezse {

Vec3 Position(const WorldMatrix& worldMatrix) {
    // Column 3 holds the translation (m[column][row]).
    const Mat4& matrix = worldMatrix.matrix;
    return Vec3{matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]};
}

Vec3 Forward(const WorldMatrix& worldMatrix) {
    return Normalize(TransformDirection(worldMatrix.matrix, Vec3{0.0f, 0.0f, -1.0f}));
}

} // namespace ezse
