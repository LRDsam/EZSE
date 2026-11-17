#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Math/Transform.h"

namespace ezse {

Mat4 ModelMatrix(const Transform& transform) {
    return Translation(transform.position) *
        Rotation(transform.rotation) *
        Scale(transform.scale);
}

Transform Interpolate(const Transform& a, const Transform& b, float t) {
    return Transform{
        Lerp(a.position, b.position, t),
        Slerp(a.rotation, b.rotation, t),
        Lerp(a.scale, b.scale, t)
    };
}

} // namespace ezse
