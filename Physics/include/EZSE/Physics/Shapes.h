#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// The shapes things collide with, in world space.

// A ball around center.
struct Sphere {
    Vec3 center;
    float radius = 0.0f;
};

// Along the axes: center and half its size on each axis, as DebugDraw::DrawBox. Rotated boxes
// come when a level has walls at an angle.
struct Box {
    Vec3 center;
    Vec3 halfExtents;
};

} // namespace ezse
