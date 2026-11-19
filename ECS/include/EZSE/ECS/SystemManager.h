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
    // Empty signature means the system will have no entities. The group is the index of a
    // SimulationGroup or PresentationGroup; World::AddSystem takes the enums.
    template <typename T, typename... Args>
    T& Add(SystemPhase systemPhase, std::size_t group, Signature signature, Args&&... args);

    // Puts the entity in every system whose signature it matches, and takes it out of the others.
    void EntitySignatureChanged(Entity entity, Signature signature);
    // The same for one system only: for a system that was just added.
    static void EntitySignatureChanged(System& system, Entity entity, Signature signature);
    void EntityDestroyed(Entity entity);

    void Update(World& world, SystemPhase systemPhase, float deltaTime);

private:
    // The systems of one group, in the order they were added.
    using SystemList = std::vector<std::unique_ptr<System>>;
    // The groups of one phase, in the order they run.
    using PhaseSystems = std::array<SystemList, c_maxSystemGroupCount>;

    std::array<PhaseSystems, c_systemPhaseCount> m_systems;
};

template <typename T, typename... Args>
T& SystemManager::Add(SystemPhase systemPhase, std::size_t group, Signature signature,
                      Args&&... args) {
    EZSE_ASSERT(group < c_maxSystemGroupCount, "System group out of range");
    std::unique_ptr<T> system = std::make_unique<T>(std::forward<Args>(args)...);
    system->m_signature = signature;
    T& systemRef = *system;
    m_systems[static_cast<std::size_t>(systemPhase)][group].push_back(std::move(system));
    return systemRef;
}

} // namespace ezse
