#pragma once

#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/EntitySet.h"
#include "EZSE/ECS/Signature.h"

#include <span>

namespace ezse {

class World;

// The base of every system: a game derives from it and adds it with World::AddSystem, which gives
// it a signature. The World keeps the list of entities that have at least those components.
class System {
public:
    virtual ~System() = default;

    // The entities that match the signature, in no fixed order. Valid until the next change of
    // the components or entities of the World: while going through it, do not add or remove
    // components of this signature and do not destroy entities; note them and do it afterwards.
    std::span<const Entity> GetEntities() const {
        return m_entities.GetEntities();
    }

protected:
    // Called once per run of its phase. deltaTime in seconds: the length of one tick in
    // SystemPhase::Simulation (always the same), the time since the last frame in
    // SystemPhase::Presentation.
    virtual void Update(World& world, float deltaTime) = 0;

private:
    friend class SystemManager;

    Signature m_signature;
    EntitySet m_entities;
};

} // namespace ezse
