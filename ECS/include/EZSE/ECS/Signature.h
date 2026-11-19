#pragma once

#include "EZSE/ECS/ComponentType.h"

#include <bitset>
#include <cstdint>

namespace ezse {

inline constexpr std::uint32_t c_maxComponentTypes = 64;

// Which components an entity has, or a system needs: one bit per component type.
using Signature = std::bitset<c_maxComponentTypes>;

// The signature of the given component types: MakeSignature<Transform, MeshRenderer>(). Without
// types it is empty.
template <typename... Ts>
Signature MakeSignature() {
    Signature signature{};
    (signature.set(ComponentType<Ts>()), ...);
    return signature;
}

} // namespace ezse
