#pragma once

#include "EZSE/Renderer/Material.h"
#include "EZSE/Renderer/Mesh.h"

namespace ezse {

// Draws a mesh with a material at the WorldMatrix of the entity. The RenderSystem draws every
// entity that has one; each also casts a shadow.
struct MeshRenderer {
    MeshHandle mesh;
    Material material;
};

} // namespace ezse
