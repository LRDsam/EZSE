#include "EZSE/Physics/Raycast.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ezse {

namespace {

// Below this a ray is parallel to a slab: dividing by it would give infinities or NaN.
constexpr float c_parallelEpsilon = 1e-8f;

// Narrows [tEnter, tExit] to where the ray is between min and max on one axis (one "slab" of the
// box). Returns false when the ray misses the slab, so it misses the box. enterAxis becomes axis
// when the ray enters this slab later than the ones before.
bool ClipToSlab(float origin, float direction, float min, float max, int axis,
                float& tEnter, float& tExit, int& enterAxis) {
    if (std::abs(direction) < c_parallelEpsilon) {
        // Parallel to the slab: inside it along the whole ray, or never.
        return origin >= min && origin <= max;
    }
    float t1 = (min - origin) / direction;
    float t2 = (max - origin) / direction;
    if (t1 > t2) {
        std::swap(t1, t2);
    }
    if (t1 > tEnter) {
        tEnter = t1;
        enterAxis = axis;
    }
    tExit = std::min(tExit, t2);
    return tEnter <= tExit;
}

} // namespace

std::optional<RaycastHit> RaycastSphere(const Ray& ray, float maxDistance, const Sphere& sphere) {
    // A point on the ray is origin + t * direction. It lies on the sphere where
    // t^2 + 2b*t + c = 0, with m = origin - center, b = Dot(m, direction) and
    // c = Dot(m, m) - radius^2 (no t^2 factor: direction has length 1).
    const Vec3 centerToOrigin = ray.origin - sphere.center;
    const float c = Dot(centerToOrigin, centerToOrigin) - sphere.radius * sphere.radius;
    if (c <= 0.0f) {
        // The origin is inside the sphere (or on it): a hit at distance 0.
        return RaycastHit{.point = ray.origin, .normal = -ray.direction, .distance = 0.0f};
    }
    const float b = Dot(centerToOrigin, ray.direction);
    if (b > 0.0f) {
        // Outside the sphere and pointing away from it.
        return std::nullopt;
    }
    const float discriminant = b * b - c;
    if (discriminant < 0.0f) {
        // The ray passes the sphere.
        return std::nullopt;
    }
    // The smaller root is where the ray enters the sphere; it is above 0 because c > 0.
    const float t = -b - std::sqrt(discriminant);
    if (t > maxDistance) {
        return std::nullopt;
    }
    const Vec3 point = ray.origin + ray.direction * t;
    return RaycastHit{.point = point,
                      .normal = (point - sphere.center) * (1.0f / sphere.radius),
                      .distance = t};
}

std::optional<RaycastHit> RaycastBox(const Ray& ray, float maxDistance, const Box& box) {
    // The slab method: the ray is inside the box where it is inside all three slabs at once, from
    // the last slab it enters (tEnter) to the first it leaves (tExit). Starting at 0 and at
    // maxDistance keeps out what lies behind the ray or too far along it.
    const Vec3 min = box.center - box.halfExtents;
    const Vec3 max = box.center + box.halfExtents;
    float tEnter = 0.0f;
    float tExit = maxDistance;
    int enterAxis = -1;
    if (!ClipToSlab(ray.origin.x, ray.direction.x, min.x, max.x, 0, tEnter, tExit, enterAxis) ||
        !ClipToSlab(ray.origin.y, ray.direction.y, min.y, max.y, 1, tEnter, tExit, enterAxis) ||
        !ClipToSlab(ray.origin.z, ray.direction.z, min.z, max.z, 2, tEnter, tExit, enterAxis)) {
        return std::nullopt;
    }

    // No slab entered after the origin: the ray starts inside the box.
    if (enterAxis == -1) {
        return RaycastHit{.point = ray.origin, .normal = -ray.direction, .distance = 0.0f};
    }

    // The normal of the face the ray enters through points against the ray on that axis: a ray
    // along +x enters through the -x face.
    Vec3 normal{};
    switch (enterAxis) {
    case 0: normal.x = ray.direction.x > 0.0f ? -1.0f : 1.0f; break;
    case 1: normal.y = ray.direction.y > 0.0f ? -1.0f : 1.0f; break;
    default: normal.z = ray.direction.z > 0.0f ? -1.0f : 1.0f; break;
    }
    return RaycastHit{.point = ray.origin + ray.direction * tEnter,
                      .normal = normal,
                      .distance = tEnter};
}

} // namespace ezse
