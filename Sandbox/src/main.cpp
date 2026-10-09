#include "EZSE/Core/Log.h"

#include "EZSE/Platform/Window.h"

#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Mesh.h"

#include "EZSE/Math/Transform.h"

#include <chrono>

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
        {{-0.25f, -0.25f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.25f, -0.25f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.25f,  0.25f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.25f,  0.25f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    };
    const std::uint32_t indices[] = { 0, 1, 2, 0, 2, 3};

    const char* vertShaderSource = R"(#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;
uniform mat4 u_transform;

out vec3 color;

void main() {
    gl_Position = u_transform * vec4(a_position, 1.0);
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

    ezse::MeshHandle mesh = renderer->CreateMesh(triangle, indices);
    ezse::ShaderHandle shader = renderer->CreateShader(vertShaderSource, fragShaderSource);

    ezse::Mat4 translation = ezse::Translation(ezse::Vec3{ 0.5f,0.5f,0.0f });
    ezse::Mat4 scale = ezse::Scale(ezse::Vec3{ 1.0f,2.0f,1.0f });
    ezse::Mat4 rotationZ = ezse::RotationZ(0.7854f);
    ezse::Mat4 proj = ezse::Perspective(0.7854f, 1280.0f / 720.0f, 0.1f, 100.0f);

    const auto start = std::chrono::steady_clock::now();
    while (!window->ShouldClose()) {
        const std::chrono::duration<float> elapsed = std::chrono::steady_clock::now() - start;
        const float seconds = elapsed.count();
        float z = -3 + std::sin(seconds);
        ezse::LogInfo("z: {}", z);
        translation = ezse::Translation(ezse::Vec3{ 0.0f,0.0f, z });


        window->PollEvents();
        renderer->Clear(0.1f, 0.2f, 0.3f);
        renderer->DrawMesh(
            mesh, 
            shader, 
            proj * translation );
        window->SwapBuffers();
    }



    ezse::LogInfo("Sandbox closed");
    return 0;
}
