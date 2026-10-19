#include "EZSE/Core/Log.h"

#include "EZSE/Platform/Window.h"

#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Mesh.h"

#include "EZSE/Math/Transform.h"
#include "EZSE/Math/Vec3.h"
#include "EZSE/Math/Vec4.h"

#include <chrono>
#include <variant>

namespace {

// Temporary: the placement of the camera, until a camera exists in the engine.
struct Transform {
    ezse::Vec3 position = ezse::Vec3{};
    float rotationY = 0.0f;
};

// Rotates a direction around the Y axis; w = 0, so only the direction turns.
ezse::Vec3 RotateVec3Y(ezse::Vec3 vec, float rotationY) {
    ezse::Vec4 vec4 = { vec.x, vec.y, vec.z, 0.0f };
    vec4 = ezse::RotationY(rotationY) * vec4;
    return ezse::Vec3{ vec4.x, vec4.y, vec4.z };
}

} // namespace

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

    // The camera begins at (0, 0, 3) and looks down the negative Z axis.
    Transform cameraTransform = Transform{};
    cameraTransform.position = ezse::Vec3{0.0f, 0.0f, 3.0f};

    // The drawing area and the projection both follow the size of the framebuffer.
    // A minimized window has size zero; then both are left as they were.
    ezse::Mat4 projection;
    const auto applyFramebufferSize = [&](int width, int height) {
        if (width == 0 || height == 0) {
            return;
        }
        const float aspect = static_cast<float>(width) / static_cast<float>(height);
        renderer->SetViewport(width, height);
        projection = ezse::Perspective(0.7854f, aspect, 0.1f, 100.0f);
    };

    const ezse::FramebufferSize framebufferSize = window->GetFramebufferSize();
    applyFramebufferSize(framebufferSize.width, framebufferSize.height);

    auto lastFrame = std::chrono::steady_clock::now();
    float cubeRotation = 0.1f;

    bool shouldClose = false;
    while (!shouldClose && !window->ShouldClose()) {
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> delta = now - lastFrame;
        lastFrame = now;
        const float seconds = delta.count();

        cubeRotation += seconds * 0.5f;
        // Two axes at different speeds, so every face comes into view.
        const ezse::Mat4 model =
            ezse::RotationY(cubeRotation * 0.5f) * ezse::RotationX(cubeRotation * 0.35f);

        window->PollEvents();
        for (const ezse::WindowEvent& event : window->GetEvents()) {
            if (const auto* resized = std::get_if<ezse::FramebufferResized>(&event)) {
                applyFramebufferSize(resized->width, resized->height);
            }
            if (const auto* key = std::get_if<ezse::KeyPressed>(&event)) {
                switch (key->key) {
                case ezse::Key::Escape:
                    shouldClose = true;
                    break;
                default:
                    break;
                }
            }
        }

        // Movement in the camera's own axes: W and S along its view, A and D sideways.
        ezse::Vec3 movement;
        if (window->IsKeyDown(ezse::Key::W)) {
            movement.z = -1.0f * seconds;
        } else if (window->IsKeyDown(ezse::Key::S)) {
            movement.z = 1.0f * seconds;
        }
        if (window->IsKeyDown(ezse::Key::A)) {
            movement.x = -1.0f * seconds;
        } else if (window->IsKeyDown(ezse::Key::D)) {
            movement.x = 1.0f * seconds;
        }

        // The arrow keys turn the camera around the Y axis.
        if (window->IsKeyDown(ezse::Key::Left)) {
            cameraTransform.rotationY += 1.0f * seconds;
        } else if (window->IsKeyDown(ezse::Key::Right)) {
            cameraTransform.rotationY -= 1.0f * seconds;
        }

        // From the camera's axes to the world's, then the view as the inverse of the placement.
        movement = RotateVec3Y(movement, cameraTransform.rotationY);
        cameraTransform.position = cameraTransform.position + movement;
        const ezse::Mat4 view = ezse::RotationY(-cameraTransform.rotationY) *
                                ezse::Translation(-cameraTransform.position);

        renderer->Clear(0.1f, 0.2f, 0.3f);
        renderer->SetCamera(view, projection);
        renderer->DrawMesh(cube, shader, model);
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
