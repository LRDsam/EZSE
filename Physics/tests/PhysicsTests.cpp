#include "EZSE/Physics/Ray.h"
#include "EZSE/Physics/Raycast.h"
#include "EZSE/Physics/Shapes.h"

#include "EZSE/Math/Vec3.h"

#include <cmath>
#include <iostream>
#include <optional>

namespace {

int failureCount = 0;

void Check(bool condition, const char* description) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failureCount;
    }
}

bool NearlyEqual(float a, float b) {
    return std::abs(a - b) < 0.0001f;
}

bool NearlyEqual(const ezse::Vec3& a, const ezse::Vec3& b) {
    return NearlyEqual(a.x, b.x) && NearlyEqual(a.y, b.y) && NearlyEqual(a.z, b.z);
}

// Straight into the screen (-Z), the way a camera looks.
const ezse::Vec3 c_forward{0.0f, 0.0f, -1.0f};

void TestRaycastSphere() {
    // A sphere of radius 1, five meters in front of the origin.
    const ezse::Sphere sphere{.center = {0.0f, 0.0f, -5.0f}, .radius = 1.0f};
    const ezse::Ray ray{.origin = {0.0f, 0.0f, 0.0f}, .direction = c_forward};

    const std::optional<ezse::RaycastHit> hit = ezse::RaycastSphere(ray, 100.0f, sphere);
    Check(hit.has_value(), "RaycastSphere: a ray straight at the sphere hits it");
    if (hit) {
        Check(NearlyEqual(hit->distance, 4.0f),
              "RaycastSphere: the distance is to the near side of the sphere");
        Check(NearlyEqual(hit->point, {0.0f, 0.0f, -4.0f}),
              "RaycastSphere: the point lies on the near side");
        Check(NearlyEqual(hit->normal, {0.0f, 0.0f, 1.0f}),
              "RaycastSphere: the normal points out of the sphere, towards the ray");
        Check(hit->entity.id == 0, "RaycastSphere: a ray against one shape has no entity");
    }

    // Hitting off center: the normal is still out of the sphere, with length 1.
    const ezse::Ray offCenter{.origin = {0.6f, 0.0f, 0.0f}, .direction = c_forward};
    const std::optional<ezse::RaycastHit> sideHit = ezse::RaycastSphere(offCenter, 100.0f, sphere);
    Check(sideHit && NearlyEqual(sideHit->distance, 4.2f) &&
              NearlyEqual(sideHit->normal, {0.6f, 0.0f, 0.8f}),
          "RaycastSphere: off center, the distance and the normal follow the curve");

    const ezse::Ray beside{.origin = {1.5f, 0.0f, 0.0f}, .direction = c_forward};
    Check(!ezse::RaycastSphere(beside, 100.0f, sphere),
          "RaycastSphere: a ray beside the sphere misses");

    const ezse::Ray away{.origin = {0.0f, 0.0f, 0.0f}, .direction = {0.0f, 0.0f, 1.0f}};
    Check(!ezse::RaycastSphere(away, 100.0f, sphere),
          "RaycastSphere: a ray pointing away from the sphere misses");

    Check(!ezse::RaycastSphere(ray, 3.0f, sphere),
          "RaycastSphere: a sphere beyond maxDistance is not hit");
    Check(ezse::RaycastSphere(ray, 4.5f, sphere).has_value(),
          "RaycastSphere: a sphere within maxDistance is hit");

    // Starting inside, also when pointing away from the center: a hit at 0.
    const ezse::Ray inside{.origin = {0.0f, 0.0f, -5.5f}, .direction = c_forward};
    const std::optional<ezse::RaycastHit> insideHit = ezse::RaycastSphere(inside, 100.0f, sphere);
    Check(insideHit && NearlyEqual(insideHit->distance, 0.0f) &&
              NearlyEqual(insideHit->point, inside.origin) &&
              NearlyEqual(insideHit->normal, {0.0f, 0.0f, 1.0f}),
          "RaycastSphere: from inside, a hit at distance 0 with the normal against the ray");
}

void TestRaycastBox() {
    // A box of 2 x 4 x 2 meters, five meters in front of the origin.
    const ezse::Box box{.center = {0.0f, 0.0f, -5.0f}, .halfExtents = {1.0f, 2.0f, 1.0f}};
    const ezse::Ray ray{.origin = {0.0f, 0.0f, 0.0f}, .direction = c_forward};

    const std::optional<ezse::RaycastHit> hit = ezse::RaycastBox(ray, 100.0f, box);
    Check(hit.has_value(), "RaycastBox: a ray straight at the box hits it");
    if (hit) {
        Check(NearlyEqual(hit->distance, 4.0f), "RaycastBox: the distance is to the near face");
        Check(NearlyEqual(hit->point, {0.0f, 0.0f, -4.0f}),
              "RaycastBox: the point lies on the near face");
        Check(NearlyEqual(hit->normal, {0.0f, 0.0f, 1.0f}),
              "RaycastBox: the normal of the +z face points along +z");
        Check(hit->entity.id == 0, "RaycastBox: a ray against one shape has no entity");
    }

    // The other faces: each normal points out of the face the ray enters through.
    const ezse::Ray fromRight{.origin = {5.0f, 0.0f, -5.0f}, .direction = {-1.0f, 0.0f, 0.0f}};
    const std::optional<ezse::RaycastHit> rightHit = ezse::RaycastBox(fromRight, 100.0f, box);
    Check(rightHit && NearlyEqual(rightHit->distance, 4.0f) &&
              NearlyEqual(rightHit->normal, {1.0f, 0.0f, 0.0f}),
          "RaycastBox: from the right, the +x face with its normal");
    const ezse::Ray fromAbove{.origin = {0.0f, 10.0f, -5.0f}, .direction = {0.0f, -1.0f, 0.0f}};
    const std::optional<ezse::RaycastHit> topHit = ezse::RaycastBox(fromAbove, 100.0f, box);
    Check(topHit && NearlyEqual(topHit->distance, 8.0f) &&
              NearlyEqual(topHit->normal, {0.0f, 1.0f, 0.0f}),
          "RaycastBox: from above, the +y face with its normal");

    // Diagonal: through the +z face, not first through the plane of a side face.
    const ezse::Vec3 diagonal = ezse::Normalize(ezse::Vec3{0.1f, 0.0f, -1.0f});
    const ezse::Ray slanted{.origin = {0.0f, 0.0f, 0.0f}, .direction = diagonal};
    const std::optional<ezse::RaycastHit> slantedHit = ezse::RaycastBox(slanted, 100.0f, box);
    Check(slantedHit && NearlyEqual(slantedHit->normal, {0.0f, 0.0f, 1.0f}) &&
              NearlyEqual(slantedHit->point.z, -4.0f),
          "RaycastBox: a slanted ray gets the face it really enters through");

    const ezse::Ray beside{.origin = {3.0f, 0.0f, 0.0f}, .direction = c_forward};
    Check(!ezse::RaycastBox(beside, 100.0f, box),
          "RaycastBox: a ray parallel to a face, outside the box, misses");

    const ezse::Ray onFacePlane{.origin = {1.0f, 0.0f, 0.0f}, .direction = c_forward};
    Check(ezse::RaycastBox(onFacePlane, 100.0f, box).has_value(),
          "RaycastBox: a ray along the plane of a face touches the box");

    const ezse::Ray away{.origin = {0.0f, 0.0f, 0.0f}, .direction = {0.0f, 0.0f, 1.0f}};
    Check(!ezse::RaycastBox(away, 100.0f, box), "RaycastBox: a ray pointing away misses");

    Check(!ezse::RaycastBox(ray, 3.0f, box), "RaycastBox: a box beyond maxDistance is not hit");

    const ezse::Ray inside{.origin = {0.0f, 0.0f, -5.0f}, .direction = {1.0f, 0.0f, 0.0f}};
    const std::optional<ezse::RaycastHit> insideHit = ezse::RaycastBox(inside, 100.0f, box);
    Check(insideHit && NearlyEqual(insideHit->distance, 0.0f) &&
              NearlyEqual(insideHit->point, inside.origin) &&
              NearlyEqual(insideHit->normal, {-1.0f, 0.0f, 0.0f}),
          "RaycastBox: from inside, a hit at distance 0 with the normal against the ray");
}

} // namespace

int main() {
    TestRaycastSphere();
    TestRaycastBox();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
