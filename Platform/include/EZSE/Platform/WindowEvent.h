#pragma once

#include "EZSE/Platform/Key.h"
#include "EZSE/Platform/MouseButton.h"

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

// A key went up.
struct KeyReleased {
    Key key = Key::Unknown;
};

// A mouse button went down. Holding it does not repeat this event.
struct MouseButtonPressed {
    MouseButton button = MouseButton::Unknown;
};

// A mouse button went up.
struct MouseButtonReleased {
    MouseButton button = MouseButton::Unknown;
};

// The mouse wheel or the touchpad scrolled, in steps: one click of a normal wheel is 1, a touchpad
// gives smaller steps. y is the normal wheel; positive is away from the user (scrolling up).
// x is sideways: a tilting wheel, or two fingers to the side on a touchpad.
struct MouseScrolled {
    float x = 0.0f;
    float y = 0.0f;
};

// One of the structs above. Add a new kind of event to this list.
using WindowEvent = std::variant<FramebufferResized,
                                 KeyPressed,
                                 KeyReleased,
                                 MouseButtonPressed,
                                 MouseButtonReleased,
                                 MouseScrolled>;

} // namespace ezse
