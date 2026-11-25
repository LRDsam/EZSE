#pragma once

#include "EZSE/ECS/ComponentManager.h"
#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/EntityManager.h"
#include "EZSE/ECS/Resource.h"
#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/SystemManager.h"
#include "EZSE/ECS/SystemPhase.h"

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace ezse {

// The entities, their components and the systems: the entrance to the ECS for a game. Not safe to
// use from two threads at once.
class World {
public:
    Entity CreateEntity();
    // Removes all its components and takes it out of every system. Must be alive (asserts).
    void DestroyEntity(Entity entity);
    bool IsAlive(Entity entity) const;

    // Once per component type, before the first AddComponent of it (asserts otherwise). The
    // Engine registers its own components (Transform, MeshRenderer and so on).
    template <typename T> void RegisterComponent();
    // The entity must be alive and must not have this component yet (asserts). Puts the entity in
    // every system whose signature it now matches.
    template <typename T> void AddComponent(Entity entity, T component);
    // The entity must have this component (asserts).
    template <typename T> void RemoveComponent(Entity entity);
    // The entity must have this component (asserts). The reference is valid until the next
    // AddComponent or RemoveComponent of the same type: do not keep it between frames.
    template <typename T> T& GetComponent(Entity entity);
    template <typename T> bool HasComponent(Entity entity) const;

    // Makes a system of type T from args, gives it the signature and the entities that already
    // match, and returns it. It runs in SystemPhase::Simulation, in the given group, after the
    // systems added to that group before it. An empty signature: the system gets no entities (for
    // work that is not per entity, such as reading the keyboard).
    template <typename T, typename... Args>
    T& AddSystem(SimulationGroup group, Signature signature, Args&&... args);
    // The same for SystemPhase::Presentation: the group sets the phase too.
    template <typename T, typename... Args>
    T& AddSystem(PresentationGroup group, Signature signature, Args&&... args);

    // Makes the one object of type T from args; the World owns it and destroys it after all
    // systems. Once per type (asserts). The reference stays valid as long as the World.
    template <typename T, typename... Args> T& AddResource(Args&&... args);
    // The resource must exist (asserts). Look it up once, when adding a system.
    template <typename T> T& GetResource();
    template <typename T> bool HasResource() const;

    // Runs the systems of one phase, group after group. The Engine calls it: Simulation once per
    // tick, Presentation once per frame.
    void Update(SystemPhase systemPhase, float deltaTime);

private:
    // Gives a system that was just added every living entity that matches its signature.
    void AddExistingEntities(System& system);

    // Declared before the systems, so it is destroyed after them: systems keep references to
    // resources.
    std::vector<std::unique_ptr<IResource>> m_resources; // index: ResourceType<T>(), may hold nulls
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
    AddExistingEntities(system);
    return system;
}

template <typename T, typename... Args>
T& World::AddSystem(PresentationGroup group, Signature signature, Args&&... args) {
    T& system = m_systems.Add<T>(SystemPhase::Presentation,
                                 static_cast<std::size_t>(group),
                                 signature,
                                 std::forward<Args>(args)...);
    AddExistingEntities(system);
    return system;
}

template <typename T, typename... Args>
T& World::AddResource(Args&&... args) {
    const std::uint32_t type = ResourceType<T>();
    if (type >= m_resources.size()) {
        m_resources.resize(type + 1);
    }
    EZSE_ASSERT(m_resources[type] == nullptr, "Cannot add the same resource type twice");
    auto resource = std::make_unique<Resource<T>>(std::forward<Args>(args)...);
    // The object stays where it is when the unique_ptr moves into the vector.
    T& value = resource->value;
    m_resources[type] = std::move(resource);
    return value;
}

template <typename T>
T& World::GetResource() {
    EZSE_ASSERT(HasResource<T>(), "Cannot get a resource that was not added");
    // Safe: the slot of ResourceType<T>() only ever holds a Resource<T>.
    return static_cast<Resource<T>&>(*m_resources[ResourceType<T>()]).value;
}

template <typename T>
bool World::HasResource() const {
    const std::uint32_t type = ResourceType<T>();
    return type < m_resources.size() && m_resources[type] != nullptr;
}

} // namespace ezse
