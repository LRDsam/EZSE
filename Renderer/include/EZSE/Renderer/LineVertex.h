#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// One end of a line for Renderer::DrawLines; two per line. The color is linear.
struct LineVertex {
    Vec3 position;
    Vec3 color;
};

} // namespace ezse
