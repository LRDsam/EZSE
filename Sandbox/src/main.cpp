#include "EZSE/Core/Log.h"
#include "EZSE/Platform/Window.h"

int main() {
    auto window = ezse::Window::Create({.title = "EZSE Sandbox"});
    if (!window) {
        return 1;
    }

    while (!window->ShouldClose()) {
        window->PollEvents();
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
