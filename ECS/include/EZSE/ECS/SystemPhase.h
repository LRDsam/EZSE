#pragma once

#include <cstddef>

namespace ezse {

// The moment in the loop at which a system runs. The Engine decides how often each phase runs.
// When adding a phase, also raise c_systemPhaseCount below.
enum class SystemPhase {
    Simulation,   // the rules of the game; on a fixed tick, the same on client and server
    Presentation  // drawing, camera, UI; once per frame, never on a server
};

inline constexpr std::size_t c_systemPhaseCount = 2;

} // namespace ezse
