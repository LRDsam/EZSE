#pragma once

#include <cstdint>

namespace ezse {

// The clock of the simulation, updated by Engine::Run. Systems that need it get a const reference
// in their constructor.
struct SimulationTime {
    std::uint64_t tick = 0;   // ticks run since the start
    float tickSeconds = 0.0f; // the length of one tick
    float alpha = 0.0f;       // how far the frame is from the last tick to the next, 0..1
};

} // namespace ezse
