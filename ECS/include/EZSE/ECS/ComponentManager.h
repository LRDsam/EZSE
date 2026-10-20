#pragma once

#include "EZSE/ECS/ComponentArray.h"
#include "EZSE/ECS/ComponentType.h"
#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/Signature.h"

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace ezse {

class ComponentManager {
public:
    template <typename T> void Register();
    template <typename T> void Add(Entity entity, T component);
    template <typename T> void Remove(Entity entity);
    template <typename T> T& Get(Entity entity);
    template <typename T> bool Has(Entity entity) const;

    void EntityDestroyed(Entity entity);
    Signature GetSignature(Entity entity) const;

private:
    template <typename T> ComponentArray<T>& GetArray();
    template <typename T> const ComponentArray<T>& GetArray() const;

    std::vector<std::unique_ptr<IComponentArray>> m_arrays;
    std::vector<Signature> m_signatures; // index: id - 1
};

template <typename T>
void ComponentManager::Register() {
    const std::uint32_t type = ComponentType<T>();
    EZSE_ASSERT(type < c_maxComponentTypes,
        "Maximum number of components Registered");
    if (type >= m_arrays.size()) {
        m_arrays.resize(type + 1);
    }
    EZSE_ASSERT(m_arrays[type] == nullptr, "Component already Registered");
    m_arrays[type] = std::make_unique<ComponentArray<T>>();
}

template <typename T>
void ComponentManager::Add(Entity entity, T component) {
    GetArray<T>().Add(entity, std::move(component));
    if (entity.id > m_signatures.size()) {
        m_signatures.resize(entity.id);
    }
    m_signatures[entity.id - 1].set(ComponentType<T>());
}

template <typename T>
void ComponentManager::Remove(Entity entity) {
    GetArray<T>().Remove(entity);
    m_signatures[entity.id - 1].reset(ComponentType<T>());
}

template <typename T>
T& ComponentManager::Get(Entity entity) {
    return GetArray<T>().Get(entity);
}

template <typename T>
bool ComponentManager::Has(Entity entity) const {
    return GetArray<T>().Has(entity);
}

template <typename T>
ComponentArray<T>& ComponentManager::GetArray() {
    const std::uint32_t type = ComponentType<T>();
    EZSE_ASSERT(
        type < m_arrays.size() &&
        m_arrays[type] != nullptr, "Component not Registered");

    IComponentArray* pointer = m_arrays[type].get();
    return *static_cast<ComponentArray<T>*>(pointer);
}

template <typename T>
const ComponentArray<T>& ComponentManager::GetArray() const {
    const std::uint32_t type = ComponentType<T>();
    EZSE_ASSERT(
        type < m_arrays.size() &&
        m_arrays[type] != nullptr, "Component not Registered");

    const IComponentArray* pointer = m_arrays[type].get();
    return *static_cast<const ComponentArray<T>*>(pointer);
}

} // namespace ezse
