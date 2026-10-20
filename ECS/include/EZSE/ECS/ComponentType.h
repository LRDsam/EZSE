#pragma once

#include <cstdint>

namespace ezse {

inline std::uint32_t NextComponentType() {
    static std::uint32_t next = 0;
    return next++;
}

template <typename T>
std::uint32_t ComponentType() {
    static const std::uint32_t type = NextComponentType();
    return type;
}

} // namespace ezse
