#pragma once

#include <cstdint>

namespace ezse {

// A number per component type, starting at 0, given in the order the types are first used. The
// same type always gets the same number within one run of the program, but two programs can give
// different numbers (the client and the server later): never send these numbers over the network
// or store them in a file.

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
