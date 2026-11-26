#include "EZSE/Physics/Ray.h"
#include "EZSE/Physics/Raycast.h"
#include "EZSE/Physics/Shapes.h"
#include "EZSE/Physics/SpatialHash.h"

#include "EZSE/Math/Aabb.h"
#include "EZSE/Math/Vec3.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

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

using Ids = std::vector<std::uint32_t>;

// A box around center, reaching half on each axis.
ezse::Aabb BoxAround(const ezse::Vec3& center, float half) {
    const ezse::Vec3 extent{half, half, half};
    return ezse::Aabb{.min = center - extent, .max = center + extent};
}

// The ids of a query, sorted: a test compares them without caring in which order the hash finds
// them.
Ids SortedQuery(ezse::SpatialHash& hash, const ezse::Aabb& bounds) {
    Ids ids;
    hash.Query(bounds, ids);
    std::sort(ids.begin(), ids.end());
    return ids;
}

void TestSpatialHashQuery() {
    ezse::SpatialHash hash(4.0f);
    hash.Insert(0, BoxAround({1.0f, 1.0f, 1.0f}, 0.5f));
    hash.Insert(1, BoxAround({50.0f, 1.0f, 1.0f}, 0.5f));

    Check(SortedQuery(hash, BoxAround({1.0f, 1.0f, 1.0f}, 1.0f)) == Ids{0},
          "SpatialHash: a query finds what lies there, and not what lies far away");
    Check(SortedQuery(hash, BoxAround({20.0f, 20.0f, 20.0f}, 1.0f)).empty(),
          "SpatialHash: a query where nothing lies finds nothing");
    Check(SortedQuery(hash, ezse::Aabb{.min = {0.0f, 0.0f, 0.0f}, .max = {60.0f, 2.0f, 2.0f}}) ==
              Ids{0, 1},
          "SpatialHash: a query over many cells finds everything in them");

    // The same cell (0, 0, 0), but the boxes do not meet: the hash itself leaves it out.
    Check(SortedQuery(hash, BoxAround({3.0f, 3.0f, 3.0f}, 0.5f)).empty(),
          "SpatialHash: an id in the same cell whose box does not overlap is left out");
    Check(SortedQuery(hash, ezse::Aabb{.min = {1.5f, 0.0f, 0.0f}, .max = {2.0f, 1.0f, 1.0f}}) ==
              Ids{0},
          "SpatialHash: boxes that only touch count as overlapping");

    Ids out{7, 8, 9};
    hash.Query(BoxAround({20.0f, 20.0f, 20.0f}, 1.0f), out);
    Check(out.empty(), "SpatialHash: Query clears out first");
}

void TestSpatialHashManyCells() {
    ezse::SpatialHash hash(4.0f);
    // A wall of 20 meters along X lies in six cells (x from -2 to 3).
    hash.Insert(3, ezse::Aabb{.min = {-6.0f, 0.0f, 0.0f}, .max = {14.0f, 3.0f, 0.2f}});

    Check(SortedQuery(hash, ezse::Aabb{.min = {-10.0f, -1.0f, -1.0f},
                                       .max = {20.0f, 4.0f, 1.0f}}) == Ids{3},
          "SpatialHash: an id in several cells is found once");
    Check(SortedQuery(hash, BoxAround({13.0f, 1.0f, 0.0f}, 0.5f)) == Ids{3},
          "SpatialHash: an id in several cells is found from its far end");
}

void TestSpatialHashNegative() {
    ezse::SpatialHash hash(4.0f);
    // On both sides of 0 on every axis, where floor and a plain cast differ.
    hash.Insert(0, BoxAround({-0.5f, -0.5f, -0.5f}, 0.25f));
    hash.Insert(1, BoxAround({0.5f, 0.5f, 0.5f}, 0.25f));
    hash.Insert(2, BoxAround({-100.0f, -30.0f, -250.0f}, 1.0f));

    Check(SortedQuery(hash, BoxAround({-0.5f, -0.5f, -0.5f}, 0.3f)) == Ids{0},
          "SpatialHash: an id just below 0 is found there");
    Check(SortedQuery(hash, BoxAround({0.5f, 0.5f, 0.5f}, 0.3f)) == Ids{1},
          "SpatialHash: an id just above 0 is found there");
    Check(SortedQuery(hash, BoxAround({0.0f, 0.0f, 0.0f}, 1.0f)) == Ids{0, 1},
          "SpatialHash: a query across 0 finds both sides");
    Check(SortedQuery(hash, BoxAround({-100.0f, -30.0f, -250.0f}, 0.5f)) == Ids{2},
          "SpatialHash: an id far into the negative is found");
}

void TestSpatialHashMove() {
    ezse::SpatialHash hash(4.0f);
    hash.Insert(0, BoxAround({1.0f, 1.0f, 1.0f}, 0.5f));

    // Within the same cell: only the box changes.
    hash.Move(0, BoxAround({3.0f, 1.0f, 1.0f}, 0.5f));
    Check(SortedQuery(hash, BoxAround({1.0f, 1.0f, 1.0f}, 0.3f)).empty(),
          "SpatialHash: after a Move within its cell, the old place is empty");
    Check(SortedQuery(hash, BoxAround({3.0f, 1.0f, 1.0f}, 0.3f)) == Ids{0},
          "SpatialHash: after a Move within its cell, it is found at the new place");

    // To cells far away.
    hash.Move(0, BoxAround({-40.0f, 1.0f, 20.0f}, 0.5f));
    Check(SortedQuery(hash, BoxAround({3.0f, 1.0f, 1.0f}, 2.0f)).empty(),
          "SpatialHash: after a Move to other cells, the old cells are empty");
    Check(SortedQuery(hash, BoxAround({-40.0f, 1.0f, 20.0f}, 0.3f)) == Ids{0},
          "SpatialHash: after a Move to other cells, it is found there");
}

void TestSpatialHashRemove() {
    ezse::SpatialHash hash(4.0f);
    // Three ids in one cell; removing the middle one moves the last into its place.
    hash.Insert(0, BoxAround({1.0f, 1.0f, 1.0f}, 0.5f));
    hash.Insert(1, BoxAround({2.0f, 1.0f, 1.0f}, 0.5f));
    hash.Insert(2, BoxAround({3.0f, 1.0f, 1.0f}, 0.5f));
    const ezse::Aabb cell{.min = {0.0f, 0.0f, 0.0f}, .max = {4.0f, 4.0f, 4.0f}};

    hash.Remove(1);
    Check(SortedQuery(hash, cell) == Ids{0, 2},
          "SpatialHash: after Remove, the other ids in its cell stay");

    hash.Remove(0);
    hash.Remove(2);
    Check(SortedQuery(hash, cell).empty(),
          "SpatialHash: after removing everything, nothing is found");

    // An id may come back, also in a cell that was emptied.
    hash.Insert(1, BoxAround({1.0f, 1.0f, 1.0f}, 0.5f));
    Check(SortedQuery(hash, cell) == Ids{1}, "SpatialHash: a removed id can be inserted again");
}

void TestSpatialHashIds() {
    ezse::SpatialHash hash(4.0f);
    // The ids do not have to come in order.
    hash.Insert(10, BoxAround({1.0f, 1.0f, 1.0f}, 0.5f));
    hash.Insert(2, BoxAround({1.5f, 1.0f, 1.0f}, 0.5f));
    Check(SortedQuery(hash, BoxAround({1.0f, 1.0f, 1.0f}, 1.0f)) == Ids{2, 10},
          "SpatialHash: ids inserted out of order are both found");

    // The stamps of one query must not hide ids from the next.
    Check(SortedQuery(hash, BoxAround({1.0f, 1.0f, 1.0f}, 1.0f)) == Ids{2, 10},
          "SpatialHash: a second query finds the same ids again");
}

} // namespace

int main() {
    TestRaycastSphere();
    TestRaycastBox();
    TestSpatialHashQuery();
    TestSpatialHashManyCells();
    TestSpatialHashNegative();
    TestSpatialHashMove();
    TestSpatialHashRemove();
    TestSpatialHashIds();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
