#pragma once

#include <memory>
#include <string>

struct GLFWwindow;

namespace ezse {

struct WindowDesc {
    std::string title = "EZSE";
    int width = 1280;
    int height = 720;
};

class Window {
public:
    static std::unique_ptr<Window> Create(const WindowDesc& desc);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    using GLProc = void (*)();
    using GLLoadFunction = GLProc (*)(const char* name);

    void PollEvents();
    void SwapBuffers();
    bool ShouldClose() const;

    GLLoadFunction GetGLLoadFunction() const;

private:
    explicit Window(GLFWwindow* handle);

    GLFWwindow* m_handle = nullptr;
};

} // namespace ezse
