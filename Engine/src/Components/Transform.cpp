#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Math/Transform.h"

namespace ezse {

Mat4 ModelMatrix(const Transform& transform) {
    return Translation(transform.position) *
        RotationY(transform.rotation.y) *
        RotationX(transform.rotation.x) *
        RotationZ(transform.rotation.z) *
        Scale(transform.scale);
}

Mat4 ViewMatrix(const Transform& transform) {
    return RotationZ(-transform.rotation.z) *
        RotationX(-transform.rotation.x) *
        RotationY(-transform.rotation.y) *
        Translation(-transform.position);
}

} // namespace ezse
