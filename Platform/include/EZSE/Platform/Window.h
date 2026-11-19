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

// How the window opens: its title and its size in screen pixels.
struct WindowDesc {
    std::string title = "EZSE";
    int width = 1280;
    int height = 720;
};

// The size of the area that is drawn to, in pixels. Can differ from the window size (a screen
// with scaling), and is 0 by 0 while the window is minimized.
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

// The window of the program with an OpenGL 4.6 context, keyboard and mouse. GLFW stays hidden
// behind it. There can be one Window at a time.
class Window {
public:
    // Opens the window and makes its OpenGL context current; vsync is on. Returns nothing (and
    // logs why) if GLFW or the window cannot be made.
    static std::unique_ptr<Window> Create(const WindowDesc& desc);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // The function the Renderer loads OpenGL with. Declared here and in the Renderer, so the two
    // modules do not know each other.
    using GLProc = void (*)();
    using GLLoadFunction = GLProc (*)(const char* name);

    // Collects what happened since the previous call; read it with GetEvents.
    void PollEvents();
    // Shows the image that was drawn; waits for the screen (vsync).
    void SwapBuffers();
    // True after the close button of the window was clicked.
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

    // Debug UI (Dear ImGui). InitDebugUI connects ImGui to the input of this window; call it once,
    // after ImGui::CreateContext. BeginDebugUIFrame passes input and size to ImGui, every frame.
    void InitDebugUI();
    void BeginDebugUIFrame();
    void ShutdownDebugUI();

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
