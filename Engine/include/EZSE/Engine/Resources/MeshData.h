#pragma once

#include "EZSE/Renderer/Mesh.h"

#include <cstdint>
#include <vector>

namespace ezse {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
};

} // namespace ezse
