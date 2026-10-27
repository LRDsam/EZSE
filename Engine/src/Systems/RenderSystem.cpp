#include "EZSE/Engine/Systems/RenderSystem.h"

#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/ECS/World.h"

namespace ezse {

RenderSystem::RenderSystem(Renderer& renderer, Vec3 clearColor)
    : m_renderer(renderer), m_clearColor(clearColor) {}

void RenderSystem::Update(World& world, float) {
    m_renderer.Clear(m_clearColor.x, m_clearColor.y, m_clearColor.z);

    for (const Entity entity : GetEntities()) {
        const Transform& transform = world.GetComponent<Transform>(entity);
        const MeshRenderer& meshRenderer = world.GetComponent<MeshRenderer>(entity);
        m_renderer.DrawMesh(meshRenderer.mesh, meshRenderer.shader, meshRenderer.texture,
            ModelMatrix(transform));
    }
}

} // namespace ezse
