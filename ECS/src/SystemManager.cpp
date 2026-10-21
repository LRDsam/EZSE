#include "EZSE/ECS/SystemManager.h"

namespace ezse {

void SystemManager::EntitySignatureChanged(Entity entity, Signature signature) {
    for (auto& system : m_systems) {
        if ((signature & system->m_signature) == system->m_signature) {
            system->m_entities.Insert(entity);
        } else {
            system->m_entities.Remove(entity);
        }
    }
}

void SystemManager::EntityDestroyed(Entity entity) {
    for (auto& system : m_systems) {
        system->m_entities.Remove(entity);
    }
}

void SystemManager::Update(World& world, float deltaTime) {
    for (auto& system : m_systems) {
        system->Update(world, deltaTime);
    }
}

} // namespace ezse
