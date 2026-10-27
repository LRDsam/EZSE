#pragma once

#include <cstdint>

namespace ezse {

// An id of 0 means "no texture".
struct TextureHandle {
    std::uint32_t id = 0;
};

} // namespace ezse
