#include "EZSE/Platform/Time.h"

#include "EZSE/Core/Log.h"

#include <ratio>
#include <thread>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif

namespace ezse {

#if defined(_WIN32)

namespace {

// Available since Windows 10 version 1803; older SDK headers do not define it.
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
    #define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

// A high-resolution waitable timer for one thread. Made on its first use in that thread and closed
// when the thread ends. Empty when Windows does not support it.
class HighResolutionTimer {
public:
    HighResolutionTimer()
        : m_handle(CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
                                          TIMER_ALL_ACCESS)) {
        if (m_handle == nullptr) {
            LogWarn("No high-resolution timer (needs Windows 10 version 1803 or newer); "
                    "sleeping is less precise");
        }
    }

    ~HighResolutionTimer() {
        if (m_handle != nullptr) {
            CloseHandle(m_handle);
        }
    }

    HighResolutionTimer(const HighResolutionTimer&) = delete;
    HighResolutionTimer& operator=(const HighResolutionTimer&) = delete;

    HANDLE Get() const { return m_handle; }

private:
    HANDLE m_handle;
};

} // namespace

void SleepUntil(std::chrono::steady_clock::time_point time) {
    const std::chrono::steady_clock::duration remaining = time - std::chrono::steady_clock::now();
    if (remaining <= std::chrono::steady_clock::duration::zero()) {
        return;
    }

    thread_local HighResolutionTimer timer;
    if (timer.Get() == nullptr) {
        std::this_thread::sleep_until(time);
        return;
    }

    // Windows counts in steps of 100 nanoseconds; a negative due time is relative to now.
    using WindowsTicks = std::chrono::duration<long long, std::ratio<1, 10'000'000>>;
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -std::chrono::duration_cast<WindowsTicks>(remaining).count();
    if (!SetWaitableTimer(timer.Get(), &dueTime, 0, nullptr, nullptr, FALSE)) {
        std::this_thread::sleep_until(time);
        return;
    }
    WaitForSingleObject(timer.Get(), INFINITE);
}

#else

void SleepUntil(std::chrono::steady_clock::time_point time) {
    // Elsewhere (Linux for the server later) the system sleep is precise to well under a
    // millisecond.
    std::this_thread::sleep_until(time);
}

#endif

} // namespace ezse
