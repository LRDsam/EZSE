#pragma once

#include "EZSE/ECS/Entity.h"
#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/System.h"
#include "EZSE/ECS/SystemPhase.h"

#include "EZSE/Core/Assert.h"

#include <array>
#include <memory>
#include <utility>
#include <vector>

namespace ezse {

class SystemManager {
public:
    // Empty signature means the system will have no entities.
    template <typename T, typename... Args>
    T& Add(SystemPhase systemPhase, Signature signature, Args&&... args);

    void EntitySignatureChanged(Entity entity, Signature signature);
    void EntityDestroyed(Entity entity);

    void Update(World& world, SystemPhase systemPhase, float deltaTime);

private:
    std::array<std::vector<std::unique_ptr<System>>, c_systemPhaseCount> m_systems;
};

template <typename T, typename... Args>
T& SystemManager::Add(SystemPhase systemPhase, Signature signature, Args&&... args) {
    std::unique_ptr<T> system = std::make_unique<T>(std::forward<Args>(args)...);
    system->m_signature = signature;
    T& systemRef = *system;
    m_systems[static_cast<std::size_t>(systemPhase)].push_back(std::move(system));
    return systemRef;
}

} // namespace ezse
