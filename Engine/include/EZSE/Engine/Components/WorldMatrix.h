#pragma once

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"

namespace ezse {

// Where an entity is drawn: its model matrix, blended between ticks for an interpolated entity.
// The WorldMatrixSystem of the Engine sets it every frame for every entity with a Transform; only
// read it.
struct WorldMatrix {
    Mat4 matrix;
};

// Where the entity is in the world: the translation column of the matrix.
Vec3 Position(const WorldMatrix& worldMatrix);
// The direction the entity looks in the world: its own -Z, of length 1 also with a scale.
Vec3 Forward(const WorldMatrix& worldMatrix);

} // namespace ezse
