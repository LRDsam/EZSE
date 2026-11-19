#pragma once

#include "EZSE/ECS/Entity.h"

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace ezse {

// The part of every ComponentArray that does not depend on T, so ComponentManager can keep them
// all in one list and tell each one that an entity was destroyed.
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

// All components of one type, as a sparse set: the components lie packed in one vector (fast to go
// through), and a second vector finds the place of an entity in O(1). Remove moves the last
// component into the hole, so the order is not fixed.
template <typename T>
class ComponentArray : public IComponentArray {
public:
    // The entity must not have this component yet (asserts).
    void Add(Entity entity, T component);
    // The entity must have this component (asserts).
    void Remove(Entity entity);
    bool Has(Entity entity) const;
    // The entity must have this component (asserts). The reference is valid until the next Add or
    // Remove in this array: an Add can move all components to new memory.
    T& Get(Entity entity);
    // Removes the component if the entity has one.
    void EntityDestroyed(Entity entity) override;

private:
    std::vector<T> m_components;
    std::vector<Entity> m_entities;       // the entity of each component, at the same place
    std::vector<std::uint32_t> m_sparse;  // index: id - 1, value: place plus one, 0 = none
};

template <typename T>
void ComponentArray<T>::Add(Entity entity, T component) {
    EZSE_ASSERT(entity.id != 0, "Cannot add a component to entity 0");
    EZSE_ASSERT(!Has(entity), "Entity already has this component");

    m_components.push_back(std::move(component));
    m_entities.push_back(entity);

    if (m_sparse.size() < entity.id) {
        m_sparse.resize(entity.id);
    }
    m_sparse[entity.id - 1] = static_cast<std::uint32_t>(m_components.size());
}

template <typename T>
void ComponentArray<T>::Remove(Entity entity) {
    EZSE_ASSERT(Has(entity), "Entity does not have this component");

    // The last component fills the hole, so the array stays packed. Not when the removed one is
    // the last itself: moving an object onto itself can empty it (a std::string, for example).
    const std::uint32_t position = m_sparse[entity.id - 1] - 1;
    const std::uint32_t last = static_cast<std::uint32_t>(m_components.size() - 1);
    if (position != last) {
        m_components[position] = std::move(m_components.back());
        m_entities[position] = m_entities.back();
        m_sparse[m_entities[position].id - 1] = position + 1;
    }
    m_sparse[entity.id - 1] = 0;

    m_components.pop_back();
    m_entities.pop_back();
}

template <typename T>
bool ComponentArray<T>::Has(Entity entity) const {
    return entity.id != 0 &&
        entity.id <= m_sparse.size() &&
        m_sparse[entity.id - 1] != 0;
}

template <typename T>
T& ComponentArray<T>::Get(Entity entity) {
    EZSE_ASSERT(Has(entity), "Entity does not have this component");
    return m_components[m_sparse[entity.id - 1] - 1];
}

template <typename T>
void ComponentArray<T>::EntityDestroyed(Entity entity) {
    if (Has(entity)) {
        Remove(entity);
    }
}

} // namespace ezse
