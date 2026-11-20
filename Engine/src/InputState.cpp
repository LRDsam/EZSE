#include "EZSE/Engine/InputState.h"

#include "EZSE/Core/Assert.h"

namespace ezse {

bool InputState::IsDown(Key key) const {
    return GetState(key).down;
}

bool InputState::IsDown(MouseButton button) const {
    return GetState(button).down;
}

bool InputState::WasPressed(Key key) const {
    return GetState(key).pressCount > 0;
}

bool InputState::WasPressed(MouseButton button) const {
    return GetState(button).pressCount > 0;
}

bool InputState::WasReleased(Key key) const {
    return GetState(key).releaseCount > 0;
}

bool InputState::WasReleased(MouseButton button) const {
    return GetState(button).releaseCount > 0;
}

int InputState::GetPressCount(Key key) const {
    return GetState(key).pressCount;
}

int InputState::GetPressCount(MouseButton button) const {
    return GetState(button).pressCount;
}

int InputState::GetReleaseCount(Key key) const {
    return GetState(key).releaseCount;
}

int InputState::GetReleaseCount(MouseButton button) const {
    return GetState(button).releaseCount;
}

MouseDelta InputState::GetMouseDelta() const {
    return m_mouseDelta;
}

ScrollDelta InputState::GetScrollDelta() const {
    return m_scrollDelta;
}

void InputState::Apply(const WindowEvent& event) {
    if (auto* keyPressed = std::get_if<KeyPressed>(&event)) {
        GetState(keyPressed->key).pressCount++;
        GetState(keyPressed->key).down = true;
        return;
    }
    if (auto* keyReleased = std::get_if<KeyReleased>(&event)) {
        GetState(keyReleased->key).releaseCount++;
        GetState(keyReleased->key).down = false;
        return;
    }
    if (auto* mouseButtonPressed = std::get_if<MouseButtonPressed>(&event)) {
        GetState(mouseButtonPressed->button).pressCount++;
        GetState(mouseButtonPressed->button).down = true;
        return;
    }
    if (auto* mouseButtonReleased = std::get_if<MouseButtonReleased>(&event)) {
        GetState(mouseButtonReleased->button).releaseCount++;
        GetState(mouseButtonReleased->button).down = false;
        return;
    }
    if (auto* mouseScrolled = std::get_if<MouseScrolled>(&event)) {
        m_scrollDelta.x += mouseScrolled->x;
        m_scrollDelta.y += mouseScrolled->y;
        return;
    }
}

void InputState::AddMouseDelta(MouseDelta delta) {
    m_mouseDelta.x += delta.x;
    m_mouseDelta.y += delta.y;
}

void InputState::ClearChanges() {
    for (ButtonState& keyState : m_keyStates) {
        keyState.pressCount = 0;
        keyState.releaseCount = 0;
    }
    for (ButtonState& buttonState : m_buttonStates) {
        buttonState.pressCount = 0;
        buttonState.releaseCount = 0;
    }
    m_scrollDelta = {};
    m_mouseDelta = {};
}

const InputState::ButtonState& InputState::GetState(Key key) const {
    EZSE_ASSERT(key < Key::Count, "GetState needs a real key");
    return m_keyStates[static_cast<std::size_t>(key)];
}

const InputState::ButtonState& InputState::GetState(MouseButton button) const {
    EZSE_ASSERT(button < MouseButton::Count, "GetState needs a real button");
    return m_buttonStates[static_cast<std::size_t>(button)];
}

InputState::ButtonState& InputState::GetState(Key key) {
    EZSE_ASSERT(key < Key::Count, "GetState needs a real key");
    return m_keyStates[static_cast<std::size_t>(key)];
}

InputState::ButtonState& InputState::GetState(MouseButton button) {
    EZSE_ASSERT(button < MouseButton::Count, "GetState needs a real button");
    return m_buttonStates[static_cast<std::size_t>(button)];
}

} // namespace ezse
