#pragma once

#include "EZSE/Core/Log.h"

// Stops in the debugger at this line. MSVC only for now; the Linux server (phase 4 of the game)
// needs another way.
#define EZSE_DEBUG_BREAK() __debugbreak()

// EZSE_ASSERT(condition, message): for programmer errors only (a handle of 0, an index out of
// range), never for what can go wrong at a user (a missing file: that gets normal error handling).
// In Debug a false condition logs the condition, the message and the place, and stops in the
// debugger. In Release the condition is not run at all, so it must not have side effects.

#if defined(EZSE_ENABLE_ASSERTS)
    #define EZSE_ASSERT(condition, message)                                    \
        do {                                                                   \
            if (!(condition)) {                                                \
                ::ezse::LogError("Assertion failed: {} - {} ({}:{})",          \
                                 #condition, message, __FILE__, __LINE__);     \
                EZSE_DEBUG_BREAK();                                            \
            }                                                                  \
        } while (false)
#else
    // sizeof does not run the condition, but it still counts as used: a loop that only asserts
    // gives no "variable not used" warning in Release.
    #define EZSE_ASSERT(condition, message) ((void)sizeof(condition))
#endif
