#pragma once

#include "EZSE/ECS/ComponentType.h"

#include <bitset>
#include <cstdint>

namespace ezse {

inline constexpr std::uint32_t c_maxComponentTypes = 64;
using Signature = std::bitset<c_maxComponentTypes>;

template <typename... Ts>
Signature MakeSignature() {
    Signature signature{};
    (signature.set(ComponentType<Ts>()), ...);
    return signature;
}

} // namespace ezse
