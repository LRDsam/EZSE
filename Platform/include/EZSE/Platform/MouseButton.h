#pragma once

namespace ezse {

// A button of the mouse. Extra buttons (the side buttons) are Unknown and give no events.
enum class MouseButton {
    Unknown,

    Left,
    Right,
    Middle,

    Count, // Not a button: the number of entries above.
};

} // namespace ezse
