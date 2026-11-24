#include "EZSE/Engine/Debug/DebugDraw.h"

#include <cmath>
#include <numbers>

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

void DebugDraw::DrawBox(const Vec3& center, const Vec3& halfExtents, const Vec3& color,
                        float duration) {
    if (!m_enabled) {
        return;
    }
    // The lowest and the highest corner; every other corner takes x, y and z from one of them.
    const Vec3 min = center - halfExtents;
    const Vec3 max = center + halfExtents;

    // The four bottom corners (y = min) and the four top corners (y = max), front is +Z.
    const Vec3 bottomFrontLeft{min.x, min.y, max.z};
    const Vec3 bottomFrontRight{max.x, min.y, max.z};
    const Vec3 bottomBackLeft{min.x, min.y, min.z};
    const Vec3 bottomBackRight{max.x, min.y, min.z};
    const Vec3 topFrontLeft{min.x, max.y, max.z};
    const Vec3 topFrontRight{max.x, max.y, max.z};
    const Vec3 topBackLeft{min.x, max.y, min.z};
    const Vec3 topBackRight{max.x, max.y, min.z};

    // The bottom.
    DrawLine(bottomFrontLeft, bottomFrontRight, color, duration);
    DrawLine(bottomFrontRight, bottomBackRight, color, duration);
    DrawLine(bottomBackRight, bottomBackLeft, color, duration);
    DrawLine(bottomBackLeft, bottomFrontLeft, color, duration);
    // The vertical edges.
    DrawLine(bottomFrontLeft, topFrontLeft, color, duration);
    DrawLine(bottomFrontRight, topFrontRight, color, duration);
    DrawLine(bottomBackRight, topBackRight, color, duration);
    DrawLine(bottomBackLeft, topBackLeft, color, duration);
    // The top.
    DrawLine(topFrontLeft, topFrontRight, color, duration);
    DrawLine(topFrontRight, topBackRight, color, duration);
    DrawLine(topBackRight, topBackLeft, color, duration);
    DrawLine(topBackLeft, topFrontLeft, color, duration);
}

void DebugDraw::DrawSphere(const Vec3& center, float radius, const Vec3& color, float duration) {
    if (!m_enabled) {
        return;
    }
    // Each ring is center + a * cos + b * sin for two of the axes, scaled to the radius. The
    // three rings share their angles, so each cos and sin is computed once instead of three times.
    const Vec3 x{radius, 0.0f, 0.0f};
    const Vec3 y{0.0f, radius, 0.0f};
    const Vec3 z{0.0f, 0.0f, radius};
    const auto pointOn = [&center](const Vec3& a, const Vec3& b, float cosine, float sine) {
        return center + a * cosine + b * sine;
    };

    // At angle 0 each ring starts at center + a.
    Vec3 previousXY = center + x;
    Vec3 previousXZ = center + x;
    Vec3 previousYZ = center + y;
    const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(c_sphereSegments);
    for (int i = 1; i <= c_sphereSegments; ++i) {
        const float angle = static_cast<float>(i) * step;
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);

        const Vec3 pointXY = pointOn(x, y, cosine, sine);
        const Vec3 pointXZ = pointOn(x, z, cosine, sine);
        const Vec3 pointYZ = pointOn(y, z, cosine, sine);
        DrawLine(previousXY, pointXY, color, duration);
        DrawLine(previousXZ, pointXZ, color, duration);
        DrawLine(previousYZ, pointYZ, color, duration);
        previousXY = pointXY;
        previousXZ = pointXZ;
        previousYZ = pointYZ;
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
