#include "EZSE/ECS/World.h"

namespace ezse {

Entity World::CreateEntity() {
    return m_entities.CreateEntity();
}

void World::DestroyEntity(Entity entity) {
    EZSE_ASSERT(m_entities.IsAlive(entity), "Entity is already destroyed");
    m_components.EntityDestroyed(entity);
    m_systems.EntityDestroyed(entity);
    m_entities.DestroyEntity(entity);
}

bool World::IsAlive(Entity entity) const {
    return m_entities.IsAlive(entity);
}

} // namespace ezse
