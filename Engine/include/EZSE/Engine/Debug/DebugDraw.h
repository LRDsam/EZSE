#pragma once

#include "EZSE/Renderer/LineVertex.h"

#include "EZSE/Math/Vec3.h"

#include <vector>

namespace ezse {

// Lines to see what the game does, such as where a raycast hits (decisions 320 to 326). Call it
// from anywhere; the Engine draws everything at the end of the frame. Without a window it does
// nothing, so the same code runs on a server.
class DebugDraw {
public:
    // enabled: false when running headless; then every Draw call returns at once.
    explicit DebugDraw(bool enabled);

    // duration 0: from a tick it stays until the next tick, from a frame only this frame.
    // A longer duration is in seconds of real time.
    void DrawLine(const Vec3& from, const Vec3& to, const Vec3& color, float duration = 0.0f);

    // For the Engine: around each tick, and once at the end of each frame.
    void BeginTick();
    void EndTick();
    void EndFrame(float frameSeconds);

    // Adds the two vertices of every line that is alive to vertices (without clearing it first).
    void CollectVertices(std::vector<LineVertex>& vertices) const;

private:
    struct TimedLine {
        LineVertex from;
        LineVertex to;
        float remaining = 0.0f; // seconds
    };

    bool m_enabled = false;
    bool m_inTick = false;
    std::vector<LineVertex> m_frameVertices; // duration 0 from a frame
    std::vector<LineVertex> m_tickVertices;  // duration 0 from a tick
    std::vector<TimedLine> m_timedLines;     // duration above 0
};

} // namespace ezse
