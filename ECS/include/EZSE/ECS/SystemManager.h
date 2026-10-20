#pragma once

#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/System.h"

#include "EZSE/Core/Assert.h"

#include <memory>
#include <utility>
#include <vector>

namespace ezse {

class SystemManager {
public:
    template <typename T, typename... Args>
    T& Add(Signature signature, Args&&... args);

    void EntitySignatureChanged(Entity entity, Signature signature);
    void EntityDestroyed(Entity entity);

private:
    std::vector<std::unique_ptr<System>> m_systems;
};

template <typename T, typename... Args>
T& SystemManager::Add(Signature signature, Args&&... args) {
    EZSE_ASSERT(!signature.none(), "Signature can't be empty");
    std::unique_ptr<T> system = std::make_unique<T>(std::forward<Args>(args)...);
    system->m_signature = signature;
    T& systemRef = *system;
    m_systems.push_back(std::move(system));
    return systemRef;
}

} // namespace ezse
