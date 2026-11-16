#pragma once

#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/ECS/Entity.h"

namespace ezse {

class World;

// The Transform at the start of the current tick. Rendering blends from this to the current
// Transform, so an entity with it moves smoothly between ticks. Add it with EnableInterpolation.
struct PreviousTransform {
    Transform transform;
};

// Turns on interpolation for an entity: adds a PreviousTransform equal to its current Transform.
// The entity must have a Transform and must not have a PreviousTransform yet (both assert).
// Only change the Transform of an interpolated entity in SystemPhase::Simulation; a change in
// Presentation skips the blend and shows as a jump.
void EnableInterpolation(World& world, Entity entity);

// Makes the previous state equal to the current one, so the next frames show no blend. Use it
// after a teleport, or the entity slides through space for one tick. Does nothing for an entity
// without a PreviousTransform.
void ResetInterpolation(World& world, Entity entity);

} // namespace ezse
