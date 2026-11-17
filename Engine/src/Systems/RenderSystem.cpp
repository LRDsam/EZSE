#include "EZSE/Engine/Systems/RenderSystem.h"

#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/WorldMatrix.h"

#include "EZSE/ECS/World.h"

namespace ezse {

RenderSystem::RenderSystem(Renderer& renderer, Vec3 clearColor)
    : m_renderer(renderer), m_clearColor(clearColor) {}

void RenderSystem::Update(World& world, float) {
    m_renderer.BeginShadowPass();
    for (const Entity entity : GetEntities()) {
        m_renderer.DrawShadowCaster(world.GetComponent<MeshRenderer>(entity).mesh,
                                    world.GetComponent<WorldMatrix>(entity).matrix);
    }
    m_renderer.EndShadowPass();

    m_renderer.Clear(m_clearColor.x, m_clearColor.y, m_clearColor.z);

    for (const Entity entity : GetEntities()) {
        const MeshRenderer& meshRenderer = world.GetComponent<MeshRenderer>(entity);
        m_renderer.DrawMesh(meshRenderer.mesh,
                            meshRenderer.material,
                            world.GetComponent<WorldMatrix>(entity).matrix);
    }
}

} // namespace ezse
