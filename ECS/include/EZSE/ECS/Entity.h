#pragma once

#include <cstdint>

namespace ezse {

// A thing in the World: only a number. What it is comes from its components. Ids start at 1 and
// are not reused after DestroyEntity.
struct Entity {
    // 0 means "no entity".
    std::uint32_t id = 0;
};

} // namespace ezse
