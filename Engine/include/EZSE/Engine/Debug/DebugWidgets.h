#pragma once

#include "EZSE/Math/Vec3.h"

namespace ezse {

// Edit a Vec3 with ImGui. Return true in the frame the value changed; the Vec3 is then updated.
// They copy through a float[3], so nothing assumes how Vec3 is laid out in memory (decision 145).
bool DragVec3(const char* label, Vec3& value, float speed);
bool ColorEditVec3(const char* label, Vec3& color);

} // namespace ezse
