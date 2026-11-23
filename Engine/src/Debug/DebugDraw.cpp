#include "EZSE/Engine/Debug/DebugDraw.h"

namespace ezse {

DebugDraw::DebugDraw(bool enabled) : m_enabled(enabled) {}

void DebugDraw::DrawLine(const Vec3& from, const Vec3& to, const Vec3& color, float duration) {
    if (!m_enabled) {
        return;
    }
    if (duration > 0.0f) {
        m_timedLines.push_back(TimedLine{.from = LineVertex{from, color},
                                         .to = LineVertex{to, color},
                                         .remaining = duration});
        return;
    }
    // Duration 0: a line from a tick must outlive the frame, or it flickers when a frame runs no
    // tick (decision 321).
    if (m_inTick) {
        m_tickVertices.push_back(LineVertex{from, color});
        m_tickVertices.push_back(LineVertex{to, color});
    } else {
        m_frameVertices.push_back(LineVertex{from, color});
        m_frameVertices.push_back(LineVertex{to, color});
    }
}

void DebugDraw::BeginTick() {
    // The lines of the previous tick are replaced by those of this one. With two ticks in one
    // frame only the last is seen, which is what the frame shows.
    m_tickVertices.clear();
    m_inTick = true;
}

void DebugDraw::EndTick() {
    m_inTick = false;
}

void DebugDraw::EndFrame(float frameSeconds) {
    m_frameVertices.clear();
    for (TimedLine& timedLine : m_timedLines) {
        timedLine.remaining -= frameSeconds;
    }
    std::erase_if(m_timedLines,
                  [](const TimedLine& timedLine) { return timedLine.remaining <= 0.0f; });
}

void DebugDraw::CollectVertices(std::vector<LineVertex>& vertices) const {
    vertices.insert(vertices.end(), m_frameVertices.begin(), m_frameVertices.end());
    vertices.insert(vertices.end(), m_tickVertices.begin(), m_tickVertices.end());
    for (const TimedLine& timedLine : m_timedLines) {
        vertices.push_back(timedLine.from);
        vertices.push_back(timedLine.to);
    }
}

} // namespace ezse
