#include "EZSE/ECS/SystemManager.h"

namespace ezse {

void SystemManager::EntitySignatureChanged(Entity entity, Signature signature) {
    for (auto& phaseSystems : m_systems) {
        for (auto& system : phaseSystems) {
            if (system->m_signature.none()) {
                continue;
            }
            if ((signature & system->m_signature) == system->m_signature) {
                system->m_entities.Insert(entity);
            } else {
                system->m_entities.Remove(entity);
            }
        }
    }
}

void SystemManager::EntityDestroyed(Entity entity) {
    for (auto& phaseSystems : m_systems) {
        for (auto& system : phaseSystems) {
            system->m_entities.Remove(entity);
        }
    }
}

void SystemManager::Update(World& world, SystemPhase systemPhase, float deltaTime) {
    for (auto& system : m_systems[static_cast<std::size_t>(systemPhase)]) {
        system->Update(world, deltaTime);
    }
}

} // namespace ezse
