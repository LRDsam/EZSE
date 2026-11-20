#pragma once

#include "EZSE/Platform/Key.h"
#include "EZSE/Platform/MouseButton.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Platform/WindowEvent.h"

#include <array>
#include <cstddef>

namespace ezse {

// How far the wheel scrolled, summed over a stretch of time; see MouseScrolled.
struct ScrollDelta {
    float x = 0.0f;
    float y = 0.0f;
};

// The input over a stretch of time: one frame, or everything since the previous tick. The Engine
// keeps one of each (Engine::GetFrameInput, Engine::GetTickInput); systems get a const reference
// in their constructor and only read.
class InputState {
public:
    // Whether the key or button is held at the end of the stretch.
    bool IsDown(Key key) const;
    bool IsDown(MouseButton button) const;
    // Whether the key or button went down at least once during the stretch. Also true for a tap
    // that went down and up again within it, when IsDown is false.
    bool WasPressed(Key key) const;
    bool WasPressed(MouseButton button) const;
    // Whether the key or button went up at least once during the stretch.
    bool WasReleased(Key key) const;
    bool WasReleased(MouseButton button) const;
    // How often the key or button went down during the stretch: two quick clicks give 2.
    int GetPressCount(Key key) const;
    int GetPressCount(MouseButton button) const;
    // How often the key or button went up during the stretch.
    int GetReleaseCount(Key key) const;
    int GetReleaseCount(MouseButton button) const;
    // How far the mouse moved during the stretch, summed.
    MouseDelta GetMouseDelta() const;
    // How far the wheel scrolled during the stretch, summed.
    ScrollDelta GetScrollDelta() const;

    // Adds one event: a press or release is counted and changes what is down, a scroll is added.
    // Other events (FramebufferResized) are ignored.
    void Apply(const WindowEvent& event);
    // Adds a movement of the mouse (Window::GetMouseDelta, once per frame).
    void AddMouseDelta(MouseDelta delta);
    // Starts a new stretch: the counts and the mouse and scroll back to 0, what is held stays held.
    void ClearChanges();

private:
    // One key or button: held or not, and how often it went down and up in this stretch.
    struct ButtonState {
        bool down = false;
        int pressCount = 0;
        int releaseCount = 0;
    };

    // The state of one key or button; asserts that it is a real one (below Count). The const
    // versions are for reading, the others for Apply and ClearChanges.
    const ButtonState& GetState(Key key) const;
    const ButtonState& GetState(MouseButton button) const;
    ButtonState& GetState(Key key);
    ButtonState& GetState(MouseButton button);

    std::array<ButtonState, static_cast<std::size_t>(Key::Count)> m_keyStates;
    std::array<ButtonState, static_cast<std::size_t>(MouseButton::Count)> m_buttonStates;
    MouseDelta m_mouseDelta;
    ScrollDelta m_scrollDelta;
};

} // namespace ezse
