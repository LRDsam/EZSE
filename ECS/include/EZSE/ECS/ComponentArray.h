#pragma once

#include "EZSE/ECS/Entity.h"

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace ezse {

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
public:
    void Add(Entity entity, T component);
    void Remove(Entity entity);
    bool Has(Entity entity) const;
    T& Get(Entity entity);
    void EntityDestroyed(Entity entity) override;

private:
    std::vector<T> m_components;
    std::vector<Entity> m_entities;
    std::vector<std::uint32_t> m_sparse;
};

template <typename T>
void ComponentArray<T>::Add(Entity entity, T component) {
    EZSE_ASSERT(entity.id != 0, "Cannot add a component to entity 0");
    EZSE_ASSERT(!Has(entity), "Entity already has component");

    m_components.push_back(std::move(component));
    m_entities.push_back(entity);

    if (m_sparse.size() < entity.id) {
        m_sparse.resize(entity.id);
    }
    m_sparse[entity.id - 1] = static_cast<std::uint32_t>(m_components.size());
}

template <typename T>
void ComponentArray<T>::Remove(Entity entity) {
    EZSE_ASSERT(Has(entity), "Entity does not have component");

    const std::uint32_t position = m_sparse[entity.id - 1] - 1;
    m_components[position] = std::move(m_components.back());
    m_entities[position] = m_entities.back();

    m_sparse[m_entities.back().id - 1] = m_sparse[entity.id - 1];
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
    EZSE_ASSERT(Has(entity), "Entity does not have component");
    return m_components[m_sparse[entity.id - 1] - 1];
}

template <typename T>
void ComponentArray<T>::EntityDestroyed(Entity entity) {
    if (Has(entity)) {
        Remove(entity);
    }
}

} // namespace ezse
