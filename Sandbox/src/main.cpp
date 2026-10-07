#include "EZSE/Core/Log.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Mesh.h"

int main() {
    auto window = ezse::Window::Create({.title = "EZSE Sandbox"});
    if (!window) {
        return 1;
    }

    auto renderer = ezse::Renderer::Create(window->GetGLLoadFunction());
    if (!renderer) {
        return 1;
    }

    const ezse::Vertex triangle[] = {
        {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    };
    const ezse::Vertex triangle2[] = {
       {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
       {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
       {{ 0.5f,  0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    };

    const char* vertShaderSource = R"(#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;

out vec3 color;

void main() {
    gl_Position = vec4(a_position, 1.0);
    color = a_color;
}
)";

    const char* fragShaderSource = R"(#version 460 core

in  vec3 color;
out vec4 fragColor;

void main() {
    fragColor = vec4(color, 1.0);
}
)";

    ezse::MeshHandle mesh = renderer->CreateMesh(triangle);
    ezse::MeshHandle mesh2 = renderer->CreateMesh(triangle2);
    ezse::ShaderHandle shader = renderer->CreateShader(vertShaderSource, fragShaderSource);

   
    while (!window->ShouldClose()) {
        window->PollEvents();
        renderer->Clear(0.1f, 0.2f, 0.3f);
        renderer->DrawMesh(mesh, shader);
        renderer->DrawMesh(mesh2, shader);
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
