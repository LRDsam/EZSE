#pragma once

#include <numbers>

namespace ezse {

// What the entity sees. It looks along its own -Z from the place of its WorldMatrix; the
// CameraSystem uses the first entity with a Camera.
struct Camera {
    float fovY = std::numbers::pi_v<float> / 4.0f; // vertical field of view in radians (45 degrees)
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};

} // namespace ezse
