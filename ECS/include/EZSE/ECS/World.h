#pragma once

#include "EZSE/ECS/ComponentManager.h"
#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/EntityManager.h"
#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/SystemManager.h"
#include "EZSE/ECS/SystemPhase.h"

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <utility>

namespace ezse {

class World {
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    bool IsAlive(Entity entity) const;

    template <typename T> void RegisterComponent();
    template <typename T> void AddComponent(Entity entity, T component);
    template <typename T> void RemoveComponent(Entity entity);
    template <typename T> T& GetComponent(Entity entity);
    template <typename T> bool HasComponent(Entity entity) const;

    template <typename T, typename... Args>
    T& AddSystem(SimulationGroup group, Signature signature, Args&&... args);
    // The group sets the phase too: a PresentationGroup runs in SystemPhase::Presentation.
    template <typename T, typename... Args>
    T& AddSystem(PresentationGroup group, Signature signature, Args&&... args);

    void Update(SystemPhase systemPhase, float deltaTime);

private:
    // Puts every living entity in the systems whose signature it matches; for a system that was
    // just added.
    void AddExistingEntitiesToSystems();

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
T& World::AddSystem(SimulationGroup group, Signature signature, Args&&... args) {
    T& system = m_systems.Add<T>(SystemPhase::Simulation,
                                 static_cast<std::size_t>(group),
                                 signature,
                                 std::forward<Args>(args)...);
    AddExistingEntitiesToSystems();
    return system;
}

template <typename T, typename... Args>
T& World::AddSystem(PresentationGroup group, Signature signature, Args&&... args) {
    T& system = m_systems.Add<T>(SystemPhase::Presentation,
                                 static_cast<std::size_t>(group),
                                 signature,
                                 std::forward<Args>(args)...);
    AddExistingEntitiesToSystems();
    return system;
}

} // namespace ezse
