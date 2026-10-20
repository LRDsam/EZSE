#pragma once

#include <bitset>
#include <cstdint>

namespace ezse {

inline constexpr std::uint32_t c_maxComponentTypes = 64;
using Signature = std::bitset<c_maxComponentTypes>;

} // namespace ezse
