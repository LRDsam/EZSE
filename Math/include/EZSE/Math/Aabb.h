#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// An axis-aligned bounding box: the smallest box along the axes around something, from its
// lowest corner to its highest. Never rotated. Used by the SpatialHash, later
// also for culling.
struct Aabb {
    Vec3 min;
    Vec3 max;
};

// True if a and b share at least one point; boxes that only touch count as overlapping.
bool Overlaps(const Aabb& a, const Aabb& b);
} // namespace ezse
