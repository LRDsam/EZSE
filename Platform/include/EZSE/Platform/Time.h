#pragma once

#include <chrono>

namespace ezse {

// Waits until the given moment. Returns at once when the moment has passed. How precise it is
// depends on the operating system (on Windows the default steps are about 1 to 15 ms).
void SleepUntil(std::chrono::steady_clock::time_point time);

} // namespace ezse
