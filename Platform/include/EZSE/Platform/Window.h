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

    GLLoadFunction GetGLLoadFunction() const;

private:
    explicit Window(GLFWwindow* handle);

    static void OnFramebufferResized(GLFWwindow* handle, int width, int height);
    static void OnKey(GLFWwindow* handle, int glfwKey, int scancode, int action, int mods);

    GLFWwindow* m_handle = nullptr;
    std::vector<WindowEvent> m_events;
    std::array<bool, static_cast<std::size_t>(Key::Count)> m_keysDown{};
};

} // namespace ezse
