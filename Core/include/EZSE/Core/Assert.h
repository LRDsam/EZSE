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
    #define EZSE_ASSERT(condition, message) ((void)0)
#endif
