#pragma once

#include <algorithm>
#include <cstddef>

namespace ezse {

// How often a system runs: the Engine decides how often each phase runs. Where a system runs
// within its phase is set by its group (SimulationGroup or PresentationGroup below).
// When adding a phase, also raise c_systemPhaseCount below.
enum class SystemPhase {
    Simulation,   // the rules of the game; on a fixed tick, the same on client and server
    Presentation  // drawing, camera, UI; once per frame, never on a server
};

inline constexpr std::size_t c_systemPhaseCount = 2;

// The order within one tick of SystemPhase::Simulation. The groups run in this order; within a
// group, systems run in the order they were added.
enum class SimulationGroup {
    PreUpdate, // the Engine: keeps the Transform of the previous tick before the game changes it
    Update     // the game: its rules, later physics and AI
};
inline constexpr std::size_t c_simulationGroupCount = 2;

// The order within one frame of SystemPhase::Presentation. The groups run in this order; within
// a group, systems run in the order they were added.
enum class PresentationGroup {
    Update,     // the game: input per frame, such as looking around
    PostUpdate, // the Engine: the WorldMatrix, after all changes of this frame
    Render      // the Engine: camera, lights and drawing, from the WorldMatrix
};
inline constexpr std::size_t c_presentationGroupCount = 3;

// The most groups any phase has; the size of the storage per phase in SystemManager.
inline constexpr std::size_t c_maxSystemGroupCount =
    std::max(c_simulationGroupCount, c_presentationGroupCount);

} // namespace ezse
