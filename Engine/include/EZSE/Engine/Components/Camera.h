#pragma once

namespace ezse {

struct Camera {
    float fovY = 0.7854f; // vertical field of view in radians (45 degrees)
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};

} // namespace ezse
