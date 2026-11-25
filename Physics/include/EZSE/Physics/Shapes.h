#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// The shapes things collide with (decision 330), in world space.

// A ball around center.
struct Sphere {
    Vec3 center;
    float radius = 0.0f;
};

// Along the axes: center and half its size on each axis, as DebugDraw::DrawBox. Rotated boxes
// come when a level has walls at an angle (decision 330).
struct Box {
    Vec3 center;
    Vec3 halfExtents;
};

} // namespace ezse
