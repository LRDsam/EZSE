#pragma once

#include "EZSE/Platform/Key.h"

#include <variant>

namespace ezse {

// The size of the area that is drawn to changed, in pixels.
struct FramebufferResized {
    int width = 0;
    int height = 0;
};

// A key went down. Holding it does not repeat this event.
struct KeyPressed {
    Key key = Key::Unknown;
};

struct KeyReleased {
    Key key = Key::Unknown;
};

// One of the structs above. Add a new kind of event to this list.
using WindowEvent = std::variant<FramebufferResized, KeyPressed, KeyReleased>;

} // namespace ezse
