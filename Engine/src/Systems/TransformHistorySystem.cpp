#include "EZSE/Engine/Systems/TransformHistorySystem.h"

#include "EZSE/Engine/Components/PreviousTransform.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/ECS/World.h"

namespace ezse {

void TransformHistorySystem::Update(World& world, float) {
    for (Entity entity : GetEntities()) {
        world.GetComponent<PreviousTransform>(entity).transform =
            world.GetComponent<Transform>(entity);
    }
}

} // namespace ezse
