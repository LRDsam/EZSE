#include "EZSE/Platform/Window.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <GLFW/glfw3.h>

namespace ezse {

namespace {

void OnGlfwError(int code, const char* description) {
    LogError("GLFW error {}: {}", code, description);
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
}

Window::~Window() {
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_handle);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_handle) != 0;
}

Window::GLLoadFunction Window::GetGLLoadFunction() const {
    return glfwGetProcAddress;
}

} // namespace ezse
