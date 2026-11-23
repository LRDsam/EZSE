#pragma once

#include "EZSE/Engine/Debug/DebugDraw.h"

#include "EZSE/ECS/System.h"
#include "EZSE/Renderer/LineVertex.h"
#include "EZSE/Renderer/Renderer.h"

#include <vector>

namespace ezse {

// Hands the lines of the DebugDraw to the Renderer, after the meshes are drawn.
class DebugDrawSystem : public System {
public:
    DebugDrawSystem(Renderer& renderer, const DebugDraw& debugDraw);

protected:
    void Update(World& world, float deltaTime) override;

private:
    Renderer& m_renderer;
    const DebugDraw& m_debugDraw;
    std::vector<LineVertex> m_vertices; // reused every frame
};

} // namespace ezse
