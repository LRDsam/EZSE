#pragma once

#include "EZSE/Math/Mat4.h"

namespace ezse {

// Where an entity is drawn: its model matrix, blended between ticks for an interpolated entity.
// The WorldMatrixSystem of the Engine sets it every frame for every entity with a Transform; only
// read it.
struct WorldMatrix {
    Mat4 matrix;
};

} // namespace ezse
