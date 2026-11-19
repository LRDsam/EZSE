#pragma once

#include "EZSE/Core/Log.h"

#define EZSE_DEBUG_BREAK() __debugbreak()

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
