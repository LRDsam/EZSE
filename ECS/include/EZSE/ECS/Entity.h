#pragma once

#include <cstdint>

namespace ezse {

struct Entity {
    // Id of zero means no entity
    std::uint32_t id = 0;
};

} // namespace ezse
