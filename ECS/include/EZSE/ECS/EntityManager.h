#pragma once

#include "EZSE/ECS/Entity.h"

#include <cstdint>
#include <vector>

namespace ezse {

// Hands out entity ids and remembers which are alive. Used by World; a game uses World.
class EntityManager {
public:
    // The next id: 1, 2, 3 and so on.
    Entity CreateEntity();
    // The entity must be alive (asserts).
    void DestroyEntity(Entity entity);
    // False for id 0, for an id that was never handed out and for a destroyed entity.
    bool IsAlive(Entity entity) const;
    // How many ids were handed out, living or not: the highest id.
    std::uint32_t GetCreatedCount() const;

private:
    std::vector<bool> m_alive;
};

} // namespace ezse
