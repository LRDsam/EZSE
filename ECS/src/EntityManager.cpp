#include "EZSE/ECS/EntityManager.h"

#include "EZSE/Core/Assert.h"

namespace ezse {

Entity EntityManager::CreateEntity() {
    m_alive.push_back(true);
    return Entity{
        static_cast<std::uint32_t>(m_alive.size())
    };
}

void EntityManager::DestroyEntity(Entity entity) {
    EZSE_ASSERT(IsAlive(entity), "Entity is not alive!");
    m_alive[entity.id - 1] = false;
}

bool EntityManager::IsAlive(Entity entity) const {
    if (entity.id > m_alive.size() || entity.id == 0) {
        return false;
    }
    return m_alive[entity.id - 1];
}

} // namespace ezse
