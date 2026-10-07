#pragma once

#include <cstdint>

namespace ezse {

// An id of 0 means "no shader".
struct ShaderHandle {
    std::uint32_t id = 0;
};

} // namespace ezse
