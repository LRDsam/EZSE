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

    const ezse::Vertex cubeVertices[] = {
        // Front (z = 0.5), red
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},

        // Back (z = -0.5), green
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},

        // Right (x = 0.5), blue
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},

        // Left (x = -0.5), yellow
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},

        // Top (y = 0.5), magenta
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},

        // Bottom (y = -0.5), cyan
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},
    };
    const std::uint32_t cubeIndices[] = {
         0,  1,  2,   0,  2,  3,   // front
         4,  5,  6,   4,  6,  7,   // back
         8,  9, 10,   8, 10, 11,   // right
        12, 13, 14,  12, 14, 15,   // left
        16, 17, 18,  16, 18, 19,   // top
        20, 21, 22,  20, 22, 23,   // bottom
    };

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

    ezse::MeshHandle cube = renderer->CreateMesh(cubeVertices, cubeIndices);
    ezse::ShaderHandle shader = renderer->CreateShader(vertShaderSource, fragShaderSource);

    // The camera stands at (0, 0, 3) and looks down the negative Z axis;
    // the view matrix is the inverse of that placement.
    const ezse::Mat4 view = ezse::Translation(ezse::Vec3{ 0.0f, 0.0f, -3.0f });
    const ezse::Mat4 projection = ezse::Perspective(0.7854f, 1280.0f / 720.0f, 0.1f, 100.0f);

    const auto start = std::chrono::steady_clock::now();
    while (!window->ShouldClose()) {
        const std::chrono::duration<float> elapsed = std::chrono::steady_clock::now() - start;
        const float seconds = elapsed.count();

        // Two axes at different speeds, so every face comes into view.
        const ezse::Mat4 model =
            ezse::RotationY(seconds * 0.5f) * ezse::RotationX(seconds * 0.35f);

        window->PollEvents();
        renderer->Clear(0.1f, 0.2f, 0.3f);
        renderer->SetCamera(view, projection);
        renderer->DrawMesh(cube, shader, model);
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
