#pragma once

namespace ezse {

// A Vec3 with a fourth part for matrix math: w = 1 for a point, w = 0 for a direction. After a
// projection, x, y and z divided by w give the place on the screen.
struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

} // namespace ezse
