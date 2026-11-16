#pragma once

#include "EZSE/Engine/SimulationTime.h"

#include "EZSE/ECS/System.h"

namespace ezse {

// Computes the WorldMatrix of every entity with a Transform, once per frame, and adds the
// component when it is missing. Blended by alpha for an entity with a PreviousTransform. The
// Engine adds it as the first system in SystemPhase::Presentation.
class WorldMatrixSystem : public System {
public:
    explicit WorldMatrixSystem(const SimulationTime& simulationTime);

protected:
    void Update(World& world, float deltaTime) override;

private:
    const SimulationTime& m_simulationTime;
};

} // namespace ezse
