#pragma once

#include "EZSE/Physics/Ray.h"
#include "EZSE/Physics/Shapes.h"

#include <optional>

namespace ezse {

// The first hit within maxDistance, or nothing. A ray that starts inside the shape hits at
// distance 0, at its origin, with the normal against its direction.
std::optional<RaycastHit> RaycastSphere(const Ray& ray, float maxDistance, const Sphere& sphere);
std::optional<RaycastHit> RaycastBox(const Ray& ray, float maxDistance, const Box& box);

} // namespace ezse
