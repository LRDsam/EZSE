#pragma once

#include "EZSE/ECS/ComponentManager.h"
#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/EntityManager.h"
#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/SystemManager.h"

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <utility>

namespace ezse {

class World {
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool  IsAlive(Entity entity) const;

    template <typename T> void RegisterComponent();
    template <typename T> void AddComponent(Entity entity, T component);
    template <typename T> void RemoveComponent(Entity entity);
    template <typename T> T& GetComponent(Entity entity);
    template <typename T> bool HasComponent(Entity entity) const;

    template <typename T, typename... Args>
    T& AddSystem(Signature signature, Args&&... args);

private:
    EntityManager m_entities;
    ComponentManager m_components;
    SystemManager m_systems;
};

template <typename T>
void World::RegisterComponent() {
    m_components.Register<T>();
}

template <typename T>
void World::AddComponent(Entity entity, T component) {
    EZSE_ASSERT(IsAlive(entity), "Cannot add a component to an entity that is not alive");
    m_components.Add<T>(entity, std::move(component));
    m_systems.EntitySignatureChanged(entity, m_components.GetSignature(entity));
}

template <typename T>
void World::RemoveComponent(Entity entity) {
    EZSE_ASSERT(IsAlive(entity), "Cannot remove a component from an entity that is not alive");
    m_components.Remove<T>(entity);
    m_systems.EntitySignatureChanged(entity, m_components.GetSignature(entity));
}

template <typename T>
T& World::GetComponent(Entity entity) {
    EZSE_ASSERT(IsAlive(entity), "Cannot get a component from an entity that is not alive");
    return m_components.Get<T>(entity);
}

template <typename T>
bool World::HasComponent(Entity entity) const {
    return m_components.Has<T>(entity);
}

template <typename T, typename... Args>
T& World::AddSystem(Signature signature, Args&&... args) {
    T& system = m_systems.Add<T>(signature, std::forward<Args>(args)...);
    for (std::uint32_t id = 1; id <= m_entities.GetCreatedCount(); ++id) {
        const Entity entity = Entity{ id };
        if (m_entities.IsAlive(entity)) {
            m_systems.EntitySignatureChanged(entity, m_components.GetSignature(entity));
        }
    }
    return system;
}

} // namespace ezse
