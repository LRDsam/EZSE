#pragma once

#include "EZSE/Renderer/Material.h"
#include "EZSE/Renderer/Mesh.h"

namespace ezse {

struct MeshRenderer {
    MeshHandle mesh;
    Material material;
};

} // namespace ezse
