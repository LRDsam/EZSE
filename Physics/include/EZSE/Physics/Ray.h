#pragma once

#include "EZSE/ECS/Entity.h"

#include "EZSE/Math/Vec3.h"

namespace ezse {

// From origin along direction, which must have length 1 (then distances are in meters).
struct Ray {
    Vec3 origin;
    Vec3 direction;
};

// Where a ray hits: the entity is empty (id 0) for a ray against one shape; the PhysicsScene
// fills it in. The normal points out of the shape, with length 1.
struct RaycastHit {
    Entity entity;
    Vec3 point;
    Vec3 normal;
    float distance = 0.0f;
};

} // namespace ezse
