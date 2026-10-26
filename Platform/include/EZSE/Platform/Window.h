#pragma once

#include "EZSE/Platform/Key.h"
#include "EZSE/Platform/WindowEvent.h"

#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>

struct GLFWwindow;

namespace ezse {

struct WindowDesc {
    std::string title = "EZSE";
    int width = 1280;
    int height = 720;
};

struct FramebufferSize {
    int width = 0;
    int height = 0;
};

// How far the mouse moved since the previous PollEvents, in pixels.
// Positive x is to the right, positive y is down (screen coordinates).
struct MouseDelta {
    float x = 0.0f;
    float y = 0.0f;
};

class Window {
public:
    static std::unique_ptr<Window> Create(const WindowDesc& desc);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    using GLProc = void (*)();
    using GLLoadFunction = GLProc (*)(const char* name);

    // Collects what happened since the previous call; read it with GetEvents.
    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;

    // The events of the last PollEvents. Valid until the next PollEvents.
    std::span<const WindowEvent> GetEvents() const;

    // The current size of the area that is drawn to, in pixels.
    FramebufferSize GetFramebufferSize() const;

    // Whether the key is held down right now.
    bool IsKeyDown(Key key) const;

    // How far the mouse moved during last PollEvents.
    MouseDelta GetMouseDelta() const;

    // Hides the cursor and keeps it in the window,
    // so the mouse can turn without limits.
    void SetCursorCaptured(bool captured);

    GLLoadFunction GetGLLoadFunction() const;

private:
    explicit Window(GLFWwindow* handle);

    static void OnFramebufferResized(GLFWwindow* handle, int width, int height);
    static void OnKey(GLFWwindow* handle, int glfwKey, int scancode, int action, int mods);

    GLFWwindow* m_handle = nullptr;
    std::vector<WindowEvent> m_events;
    std::array<bool, static_cast<std::size_t>(Key::Count)> m_keysDown{};
    double m_lastCursorX = 0.0;
    double m_lastCursorY = 0.0;
    MouseDelta m_mouseDelta;
};

} // namespace ezse
