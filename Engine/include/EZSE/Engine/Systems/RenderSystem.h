#pragma once

#include "EZSE/ECS/System.h"
#include "EZSE/Math/Vec3.h"
#include "EZSE/Renderer/Renderer.h"

namespace ezse {

// Clears the screen and draws every entity with a Transform and a MeshRenderer.
class RenderSystem : public System {
public:
    RenderSystem(Renderer& renderer, Vec3 clearColor);

protected:
    void Update(World& world, float deltaTime) override;

private:
    Renderer& m_renderer;
    Vec3 m_clearColor; // red, green and blue in x, y and z
};

} // namespace ezse
