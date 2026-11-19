#include "EZSE/Engine/Components/PreviousTransform.h"

#include "EZSE/ECS/World.h"

namespace ezse {

void EnableInterpolation(World& world, Entity entity) {
    world.AddComponent<PreviousTransform>(
        entity,
        PreviousTransform{world.GetComponent<Transform>(entity)});
}

void ResetInterpolation(World& world, Entity entity) {
    if (!world.HasComponent<PreviousTransform>(entity)) {
        return;
    }

    PreviousTransform& prevTransform = world.GetComponent<PreviousTransform>(entity);
    prevTransform.transform = world.GetComponent<Transform>(entity);
}

} // namespace ezse
