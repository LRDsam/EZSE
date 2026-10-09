#include "EZSE/Core/Log.h"

#include "EZSE/Platform/Window.h"

#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Mesh.h"

#include "EZSE/Math/Transform.h"

#include <chrono>
#include <cmath>

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
layout(std140, binding = 0) uniform Camera {
    mat4 u_viewProjection;
};
uniform mat4 u_model;

out vec3 color;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 1.0);
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

    const ezse::Mat4 model =
        ezse::Translation(ezse::Vec3{ 0.0f, 0.0f, 0.0f }) * ezse::RotationZ(0.7854f);
    const ezse::Mat4 projection = ezse::Perspective(0.7854f, 1280.0f / 720.0f, 0.1f, 100.0f);

    const ezse::Vec3 cameraPos{ 0.0f, 0.0f, 3.0f };

    const auto start = std::chrono::steady_clock::now();
    while (!window->ShouldClose()) {
        const std::chrono::duration<float> elapsed = std::chrono::steady_clock::now() - start;
        const float seconds = elapsed.count();
        const float cameraYaw = std::sin(seconds) * 0.5f;

        // The camera is placed with Translation(cameraPos) * RotationY(cameraYaw).
        // The view matrix undoes that: both parts inverted, in the opposite order.
        const ezse::Mat4 view =
            ezse::RotationY(-cameraYaw) *
            ezse::Translation(ezse::Vec3{ -cameraPos.x, -cameraPos.y, -cameraPos.z });

        window->PollEvents();
        renderer->Clear(0.1f, 0.2f, 0.3f);
        renderer->SetCamera(view, projection);
        renderer->DrawMesh(mesh, shader, model);
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
