#pragma once

#include <cstdint>

namespace ezse {

// One corner of a mesh, as the shaders read it: attribute 0 is the position, 1 the normal (length
// 1, pointing out of the surface), 2 the texture coordinate (0 to 1, (0, 0) is bottom left).
struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];
};

// An id of 0 means "no mesh".
struct MeshHandle {
    std::uint32_t id = 0;
};

} // namespace ezse
