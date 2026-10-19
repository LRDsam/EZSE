#pragma once

#include "EZSE/ECS/Entity.h"

#include <vector> 

namespace ezse {

class EntityManager {
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsAlive(Entity entity) const;
private:
    std::vector<bool> m_alive;
};

} // namespace ezse
