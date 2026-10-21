#pragma once

#include "EZSE/ECS/Entity.h"

#include <vector>

namespace ezse {

class EntityManager {
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsAlive(Entity entity) const;
    std::uint32_t GetCreatedCount() const;
private:
    std::vector<bool> m_alive;
};

} // namespace ezse
