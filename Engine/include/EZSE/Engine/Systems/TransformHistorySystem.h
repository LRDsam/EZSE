#pragma once

#include "EZSE/ECS/System.h"

namespace ezse {

// Copies the Transform to the PreviousTransform of every interpolated entity. The Engine adds it
// as the first system in SystemPhase::Simulation, so the copy is the state at the start of a tick.
class TransformHistorySystem : public System {
protected:
    void Update(World& world, float deltaTime) override;
};

} // namespace ezse
