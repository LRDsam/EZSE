#pragma once

#include <chrono>

namespace ezse {

// Waits until the given moment. Returns at once when the moment has passed. On Windows it uses a
// high-resolution timer (under 1 ms late; the plain sleep of Windows can be up to 16 ms late);
// elsewhere the sleep of the system, which is already precise.
void SleepUntil(std::chrono::steady_clock::time_point time);

} // namespace ezse
