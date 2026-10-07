#pragma once

#include <cstdint>

namespace ezse {

struct Vertex {
    float position[3];
    float color[3];
};

// An id of 0 means "no mesh".
struct MeshHandle {
    std::uint32_t id = 0;
};

} // namespace ezse
