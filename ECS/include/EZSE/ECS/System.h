#pragma once

#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/EntitySet.h"
#include "EZSE/ECS/Signature.h"

#include <span>

namespace ezse {

class World;

class System {
public:
    virtual ~System() = default;

    std::span<const Entity> GetEntities() const {
        return m_entities.GetEntities();
    }

protected:
    virtual void Update(World& world, float deltaTime) = 0;

private:
    friend class SystemManager;

    Signature m_signature;
    EntitySet m_entities;
};

} // namespace ezse
