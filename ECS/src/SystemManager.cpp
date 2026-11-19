#include "EZSE/ECS/SystemManager.h"

namespace ezse {

void SystemManager::EntitySignatureChanged(Entity entity, Signature signature) {
    for (PhaseSystems& phase : m_systems) {
        for (SystemList& group : phase) {
            for (auto& system : group) {
                EntitySignatureChanged(*system, entity, signature);
            }
        }
    }
}

void SystemManager::EntitySignatureChanged(System& system, Entity entity, Signature signature) {
    // An empty signature means the system has no entities, not that every entity matches.
    if (system.m_signature.none()) {
        return;
    }
    if ((signature & system.m_signature) == system.m_signature) {
        system.m_entities.Insert(entity);
    } else {
        system.m_entities.Remove(entity);
    }
}

void SystemManager::EntityDestroyed(Entity entity) {
    for (PhaseSystems& phase : m_systems) {
        for (SystemList& group : phase) {
            for (auto& system : group) {
                system->m_entities.Remove(entity);
            }
        }
    }
}

void SystemManager::Update(World& world, SystemPhase systemPhase, float deltaTime) {
    for (SystemList& group : m_systems[static_cast<std::size_t>(systemPhase)]) {
        for (auto& system : group) {
            system->Update(world, deltaTime);
        }
    }
}

} // namespace ezse
