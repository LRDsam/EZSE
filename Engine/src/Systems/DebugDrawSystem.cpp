#include "EZSE/Engine/Systems/DebugDrawSystem.h"

namespace ezse {

DebugDrawSystem::DebugDrawSystem(Renderer& renderer, const DebugDraw& debugDraw) :
    m_renderer(renderer), m_debugDraw(debugDraw) {}

void DebugDrawSystem::Update(World&, float) {
    // Cleared, not made anew: the vector keeps its memory from frame to frame.
    m_vertices.clear();
    m_debugDraw.CollectVertices(m_vertices);
    m_renderer.DrawLines(m_vertices);
}

} // namespace ezse
