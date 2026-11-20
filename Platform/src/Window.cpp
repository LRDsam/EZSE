#include "EZSE/Platform/Window.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

namespace ezse {

namespace {

void OnGlfwError(int code, const char* description) {
    LogError("GLFW error {}: {}", code, description);
}

// Letters, digits and function keys are consecutive in both GLFW and Key,
// so those ranges are translated with an offset instead of a case per key.
// The checks below fail to compile when a range in Key gets a gap or a different length.
static_assert(static_cast<int>(Key::Z) - static_cast<int>(Key::A) == GLFW_KEY_Z - GLFW_KEY_A);
static_assert(static_cast<int>(Key::Digit9) - static_cast<int>(Key::Digit0) ==
              GLFW_KEY_9 - GLFW_KEY_0);
static_assert(static_cast<int>(Key::F12) - static_cast<int>(Key::F1) == GLFW_KEY_F12 - GLFW_KEY_F1);

Key ToKey(int glfwKey) {
    if (glfwKey >= GLFW_KEY_A && glfwKey <= GLFW_KEY_Z) {
        return static_cast<Key>(static_cast<int>(Key::A) + (glfwKey - GLFW_KEY_A));
    }
    if (glfwKey >= GLFW_KEY_0 && glfwKey <= GLFW_KEY_9) {
        return static_cast<Key>(static_cast<int>(Key::Digit0) + (glfwKey - GLFW_KEY_0));
    }
    if (glfwKey >= GLFW_KEY_F1 && glfwKey <= GLFW_KEY_F12) {
        return static_cast<Key>(static_cast<int>(Key::F1) + (glfwKey - GLFW_KEY_F1));
    }

    switch (glfwKey) {
        case GLFW_KEY_SPACE:         return Key::Space;
        case GLFW_KEY_ESCAPE:        return Key::Escape;
        case GLFW_KEY_ENTER:         return Key::Enter;
        case GLFW_KEY_TAB:           return Key::Tab;
        case GLFW_KEY_BACKSPACE:     return Key::Backspace;
        case GLFW_KEY_LEFT:          return Key::Left;
        case GLFW_KEY_RIGHT:         return Key::Right;
        case GLFW_KEY_UP:            return Key::Up;
        case GLFW_KEY_DOWN:          return Key::Down;
        case GLFW_KEY_LEFT_SHIFT:    return Key::LeftShift;
        case GLFW_KEY_RIGHT_SHIFT:   return Key::RightShift;
        case GLFW_KEY_LEFT_CONTROL:  return Key::LeftControl;
        case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
        case GLFW_KEY_LEFT_ALT:      return Key::LeftAlt;
        case GLFW_KEY_RIGHT_ALT:     return Key::RightAlt;
        default:                     return Key::Unknown;
    }
}

MouseButton ToMouseButton(int glfwMouseButton) {
    switch (glfwMouseButton) {
        case GLFW_MOUSE_BUTTON_LEFT:    return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:   return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE:  return MouseButton::Middle;
        default:                        return MouseButton::Unknown;
    }
}

} // namespace

std::unique_ptr<Window> Window::Create(const WindowDesc& desc) {
    glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit()) {
        LogError("Failed to initialize GLFW");
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
#if defined(EZSE_DEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    GLFWwindow* handle =
        glfwCreateWindow(desc.width, desc.height, desc.title.c_str(), nullptr, nullptr);
    if (!handle) {
        LogError("Failed to create window '{}'", desc.title);
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(handle);
    glfwSwapInterval(1);

    LogInfo("Created window '{}' ({}x{}) with OpenGL {}.{} context", desc.title, desc.width,
            desc.height, glfwGetWindowAttrib(handle, GLFW_CONTEXT_VERSION_MAJOR),
            glfwGetWindowAttrib(handle, GLFW_CONTEXT_VERSION_MINOR));
    return std::unique_ptr<Window>(new Window(handle));
}

Window::Window(GLFWwindow* handle) : m_handle(handle) {
    EZSE_ASSERT(m_handle != nullptr, "Window constructed without a GLFW handle");

    // GLFW calls plain functions; this pointer lets them find the Window again.
    glfwSetWindowUserPointer(m_handle, this);
    glfwSetFramebufferSizeCallback(m_handle, OnFramebufferResized);
    glfwSetKeyCallback(m_handle, OnKey);
    glfwSetMouseButtonCallback(m_handle, OnMouseButton);
    glfwSetScrollCallback(m_handle, OnScroll);

    // Setting the start position of the mouse.
    glfwGetCursorPos(m_handle, &m_lastCursorX, &m_lastCursorY);
}

Window::~Window() {
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void Window::PollEvents() {
    m_events.clear();
    glfwPollEvents();
    double cursorX = 0.0;
    double cursorY = 0.0;
    glfwGetCursorPos(m_handle, &cursorX, &cursorY);
    m_mouseDelta.x = static_cast<float>(cursorX - m_lastCursorX);
    m_mouseDelta.y = static_cast<float>(cursorY - m_lastCursorY);
    // Kept as double: with a captured cursor GLFW keeps adding to the position, and a float
    // would lose the small steps after a long time.
    m_lastCursorX = cursorX;
    m_lastCursorY = cursorY;
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_handle);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_handle) != 0;
}

std::span<const WindowEvent> Window::GetEvents() const {
    return m_events;
}

FramebufferSize Window::GetFramebufferSize() const {
    FramebufferSize size;
    glfwGetFramebufferSize(m_handle, &size.width, &size.height);
    return size;
}

bool Window::IsKeyDown(Key key) const {
    EZSE_ASSERT(key < Key::Count, "IsKeyDown needs a real key");
    return m_keysDown[static_cast<std::size_t>(key)];
}

MouseDelta Window::GetMouseDelta() const {
    return m_mouseDelta;
}

bool Window::IsMouseButtonDown(MouseButton button) const {
    EZSE_ASSERT(button < MouseButton::Count, "IsMouseButtonDown needs a real Button");
    return m_buttonsDown[static_cast<std::size_t>(button)];
}

void Window::SetCursorCaptured(bool captured) {
    glfwSetInputMode(m_handle, GLFW_CURSOR,
        captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_handle, GLFW_RAW_MOUSE_MOTION,
            captured ? GLFW_TRUE : GLFW_FALSE);
    }
    glfwGetCursorPos(m_handle, &m_lastCursorX, &m_lastCursorY);
}

bool Window::IsCursorCaptured() const {
    return glfwGetInputMode(m_handle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

void Window::InitDebugUI() {
    // true: ImGui installs its own GLFW callbacks and calls ours after them.
    ImGui_ImplGlfw_InitForOpenGL(m_handle, true);
}

void Window::BeginDebugUIFrame() {
    ImGui_ImplGlfw_NewFrame();
}

void Window::ShutdownDebugUI() {
    ImGui_ImplGlfw_Shutdown();
}

Window::GLLoadFunction Window::GetGLLoadFunction() const {
    return glfwGetProcAddress;
}

void Window::OnFramebufferResized(GLFWwindow* handle, int width, int height) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    window->m_events.push_back(FramebufferResized{width, height});
}

void Window::OnKey(GLFWwindow* handle, int glfwKey, int, int action, int) {
    const Key key = ToKey(glfwKey);
    if (key == Key::Unknown || action == GLFW_REPEAT) {
        return;
    }

    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    const bool pressed = action == GLFW_PRESS;
    window->m_keysDown[static_cast<std::size_t>(key)] = pressed;

    if (pressed) {
        window->m_events.push_back(KeyPressed{key});
    } else {
        window->m_events.push_back(KeyReleased{key});
    }
}

void Window::OnMouseButton(GLFWwindow* handle, int glfwButton, int action, int) {
    const MouseButton button = ToMouseButton(glfwButton);
    if (button == MouseButton::Unknown) {
        return;
    }

    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    const bool pressed = action == GLFW_PRESS;
    window->m_buttonsDown[static_cast<std::size_t>(button)] = pressed;

    if (pressed) {
        window->m_events.push_back(MouseButtonPressed{button});
    } else {
        window->m_events.push_back(MouseButtonReleased{button});
    }
}

void Window::OnScroll(GLFWwindow* handle, double xOffset, double yOffset) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
    window->m_events.push_back(
        MouseScrolled{static_cast<float>(xOffset), static_cast<float>(yOffset)});
}

} // namespace ezse
