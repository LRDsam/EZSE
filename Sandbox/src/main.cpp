#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"
#include "EZSE/Engine/Engine.h"
#include "EZSE/Engine/Resources/Image.h"
#include "EZSE/Engine/Systems/CameraSystem.h"
#include "EZSE/Engine/Systems/RenderSystem.h"

#include "EZSE/Core/Log.h"
#include "EZSE/Core/File.h"

#include "EZSE/Renderer/Mesh.h"

#include "EZSE/Platform/Window.h"
#include "EZSE/Platform/Key.h"
#include "EZSE/Platform/WindowEvent.h"
#include "EZSE/Platform/FileSystem.h"

#include "EZSE/Math/Transform.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <numbers>
#include <optional>
#include <string>
#include <variant>

namespace {

// Components
// Turns an entity at a fixed speed
struct Spin {
    ezse::Vec3 speed; // Radians per second around X, Y, Z
};

// Lets the keyboard and the mouse move and turn an entity.
struct CameraController {
    float moveSpeed = 1.0f; // units per second.
    float lookSensitivity = 0.002f; // radians per pixel of mouse movement.
};

// Systems
class GameControlSystem : public ezse::System {
public:
    GameControlSystem(ezse::Window& window, ezse::Engine& engine);

protected:
    void Update(ezse::World& world, float deltaTime) override;

private:
    ezse::Window& m_window;
    ezse::Engine& m_engine;
};

GameControlSystem::GameControlSystem(ezse::Window& window, ezse::Engine& engine) :
    m_window(window), m_engine(engine) {}

void GameControlSystem::Update(ezse::World&, float) {
    for (const ezse::WindowEvent& event : m_window.GetEvents()) {
        if (const ezse::KeyPressed* pressed =
            std::get_if<ezse::KeyPressed>(&event)) {
            if (pressed->key == ezse::Key::Escape) {
                m_engine.Quit();
            }
        }
    }
}

// Adds the Spin speed of each entity to the rotation of its Transform.
class SpinSystem : public ezse::System {
protected:
    void Update(ezse::World& world, float deltaTime) override;
};

void SpinSystem::Update(ezse::World& world, float deltaTime) {
    for (ezse::Entity entity : GetEntities()) {
        ezse::Transform& transform = world.GetComponent<ezse::Transform>(entity);
        const Spin& spin = world.GetComponent<Spin>(entity);
        transform.rotation = transform.rotation + spin.speed * deltaTime;
    }
}


// Enables control of the entity by user input.
// Mouse movement to look around and WASD to move.
class CameraControlSystem : public ezse::System {
public:
    explicit CameraControlSystem(ezse::Window& window);

protected:
    void Update(ezse::World& world, float deltaTime) override;

private:
    ezse::Window& m_window;
};

CameraControlSystem::CameraControlSystem(ezse::Window& window) :
    m_window(window) {}

void CameraControlSystem::Update(ezse::World& world, float deltaTime) {
    const float maxPitch = 89.0f * (std::numbers::pi_v<float> / 180.0f);

    ezse::Vec3 move;
    if (m_window.IsKeyDown(ezse::Key::W)) {
        move.z -= 1.0f;
    }
    if (m_window.IsKeyDown(ezse::Key::S)) {
        move.z += 1.0f;
    }
    if (m_window.IsKeyDown(ezse::Key::D)) {
        move.x += 1.0f;
    }
    if (m_window.IsKeyDown(ezse::Key::A)) {
        move.x -= 1.0f;
    }
    move = ezse::Normalize(move);
    const ezse::MouseDelta mouseDelta = m_window.GetMouseDelta();
    for (ezse::Entity entity : GetEntities()) {
        ezse::Transform& transform = world.GetComponent<ezse::Transform>(entity);
        const CameraController& cameraController = world.GetComponent<CameraController>(entity);
        transform.rotation.y -= (mouseDelta.x * cameraController.lookSensitivity);
        transform.rotation.x -= (mouseDelta.y * cameraController.lookSensitivity);
        transform.rotation.x = std::clamp(transform.rotation.x, -maxPitch, maxPitch);
        const ezse::Vec3 moveEntity = ezse::TransformDirection(
            ezse::RotationY(transform.rotation.y),
            move);
        transform.position =
            transform.position + (moveEntity * (cameraController.moveSpeed * deltaTime));
    }
}

} // namespace

int main() {
    const ezse::Vertex cubeVertices[] = {
        // Front (z = 0.5), normal +Z
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

        // Back (z = -0.5), normal -Z
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

        // Right (x = 0.5), normal +X
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

        // Left (x = -0.5), normal -X
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

        // Top (y = 0.5), normal +Y
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},

        // Bottom (y = -0.5), normal -Y
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},
    };
    const std::uint32_t cubeIndices[] = {
         0,  1,  2,   0,  2,  3,   // front
         4,  5,  6,   4,  6,  7,   // back
         8,  9, 10,   8, 10, 11,   // right
        12, 13, 14,  12, 14, 15,   // left
        16, 17, 18,  16, 18, 19,   // top
        20, 21, 22,  20, 22, 23,   // bottom
    };

    const std::filesystem::path shaderDir =
        ezse::GetExecutableDirectory() / "assets" / "shaders";
    const std::filesystem::path textureDir =
        ezse::GetExecutableDirectory() / "assets" / "textures";

    const std::optional<std::string> vertexSource = ezse::ReadTextFile(
        shaderDir / "lit.vert");
    const std::optional<std::string> fragmentSource = ezse::ReadTextFile(
        shaderDir / "lit.frag");
    if (!vertexSource.has_value() || !fragmentSource.has_value()) {
        ezse::LogError("Reading vertex and/or fragment source failed.");
        return 1;
    }

    const std::optional<ezse::Image> checkerImage = ezse::ReadImageFile(textureDir / "checker.png");
    if (!checkerImage.has_value()) {
        return 1;
    }

    auto engine = ezse::Engine::Create({.title = "EZSE Sandbox"});
    if (!engine) {
        return 1;
    }

    ezse::Renderer& renderer = engine->GetRenderer();

    const ezse::TextureHandle checkerTexture = renderer.CreateTexture(
        checkerImage->width, checkerImage->height, checkerImage->pixels);
    const ezse::MeshHandle cubeMesh = renderer.CreateMesh(cubeVertices, cubeIndices);
    const ezse::ShaderHandle shader = renderer.CreateShader(*vertexSource,
        *fragmentSource);
    if (shader.id == 0) {
        ezse::LogError("Failed to compile shader.");
        return 1;
    }
    renderer.SetDirectionalLight(
        ezse::Vec3{ -0.5f, -1.0f, -0.7f },
        ezse::Vec3{ 1.0f, 1.0f, 1.0f });
    renderer.SetAmbientLight(ezse::Vec3{ 0.15f, 0.15f, 0.15f });

    ezse::World& world = engine->GetWorld();

    // Registering components
    world.RegisterComponent<Spin>();
    world.RegisterComponent<CameraController>();

    // Creating the entities
    const ezse::Entity cube = world.CreateEntity();
    world.AddComponent(cube, ezse::Transform{0.0f, -2.0f, 0.0f});
    world.AddComponent(cube, ezse::MeshRenderer{cubeMesh, {.shader = shader, .texture = checkerTexture}});
    //world.AddComponent(cube, Spin{ .speed{0.175f, 0.25f, 0.0f} });
    // A second cube with the same mesh: no texture (white), an orange tint and a different shine.
    const ezse::Entity orangeCube = world.CreateEntity();
    world.AddComponent(orangeCube, ezse::Transform{1.5f, -2.0f, 0.0f});
    world.AddComponent(orangeCube, ezse::MeshRenderer{cubeMesh, {
        .shader = shader,
        .tint = {1.0f, 0.5f, 0.1f},
        .shininess = 16.0f,
        .specularStrength = 1.0f}});
    // The camera begins at (0, 0, 3) and looks down the negative Z axis.
    const ezse::Entity camera = world.CreateEntity();
    world.AddComponent(camera, ezse::Transform{.position = {0.0f, 0.0f, 3.0f}});
    world.AddComponent(camera, ezse::Camera{});
    world.AddComponent(camera, CameraController{});

    // Registing the systems
    world.AddSystem<GameControlSystem>(ezse::MakeSignature<>(),
        engine->GetWindow(), *engine);
    world.AddSystem<SpinSystem>(ezse::MakeSignature<ezse::Transform, Spin>());
    world.AddSystem<CameraControlSystem>(
        ezse::MakeSignature<ezse::Transform, CameraController>(),
        engine->GetWindow());
    // The engine systems come last, the camera before the drawing (decision 115).
    world.AddSystem<ezse::CameraSystem>(ezse::MakeSignature<ezse::Transform, ezse::Camera>(),
                                        engine->GetWindow(), renderer);
    world.AddSystem<ezse::RenderSystem>(ezse::MakeSignature<ezse::Transform, ezse::MeshRenderer>(),
                                        renderer, ezse::Vec3{0.1f, 0.2f, 0.3f});

    engine->GetWindow().SetCursorCaptured(true);
    engine->Run();

    ezse::LogInfo("Sandbox closed");
    return 0;
}
