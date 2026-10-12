#pragma once

#include <variant>

namespace ezse {

// The size of the area that is drawn to changed, in pixels.
struct FramebufferResized {
    int width = 0;
    int height = 0;
};

// One of the structs above. Add a new kind of event to this list.
using WindowEvent = std::variant<FramebufferResized>;

} // namespace ezse
