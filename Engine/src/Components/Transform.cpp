#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Math/Transform.h"

namespace ezse {

Mat4 ModelMatrix(const Transform& transform) {
    return Translation(transform.position) *
        Rotation(transform.rotation) *
        Scale(transform.scale);
}

Mat4 ViewMatrix(const Transform& transform) {
    // The inverse of the camera's own placement: move back, then turn back with the Conjugate.
    return Rotation(Conjugate(transform.rotation)) *
        Translation(-transform.position);
}

} // namespace ezse
