#include "EZSE/Engine/Systems/WorldMatrixSystem.h"

#include "EZSE/Engine/Components/PreviousTransform.h"
#include "EZSE/Engine/Components/Transform.h"
#include "EZSE/Engine/Components/WorldMatrix.h"

#include "EZSE/ECS/World.h"

namespace ezse {

WorldMatrixSystem::WorldMatrixSystem(const SimulationTime& simulationTime) :
    m_simulationTime(simulationTime) {}

void WorldMatrixSystem::Update(World& world, float) {
    for (Entity entity : GetEntities()) {
        const Transform& transform = world.GetComponent<Transform>(entity);
        if (!world.HasComponent<WorldMatrix>(entity)) {
            world.AddComponent<WorldMatrix>(entity, WorldMatrix{});
        }
        if (world.HasComponent<PreviousTransform>(entity)) {
            const Transform& prevTransform = world.GetComponent<PreviousTransform>(entity).transform;
            world.GetComponent<WorldMatrix>(entity).matrix =
                ModelMatrix(Interpolate(prevTransform, transform, m_simulationTime.alpha));
            continue;
        }
        world.GetComponent<WorldMatrix>(entity).matrix = ModelMatrix(transform);
    }
}

} // namespace ezse
