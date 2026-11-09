#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/Light.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"
#include "EZSE/Engine/Engine.h"
#include "EZSE/Engine/Debug/DebugUI.h"
#include "EZSE/Engine/Debug/DebugWidgets.h"

#include "EZSE/Engine/Systems/CameraSystem.h"
#include "EZSE/Engine/Systems/LightSystem.h"
#include "EZSE/Engine/Systems/RenderSystem.h"

#include "EZSE/Core/Log.h"

#include "EZSE/Renderer/Mesh.h"

#include "EZSE/Platform/Window.h"
#include "EZSE/Platform/Key.h"
#include "EZSE/Platform/WindowEvent.h"

#include "EZSE/Math/Transform.h"

#include <algorithm>
#include <cstdint>
#include <numbers>
#include <variant>

#include <imgui.h>

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
            // F1 shows or hides the debug UI; the cursor is free while it is shown.
            if (pressed->key == ezse::Key::F1) {
                ezse::DebugUI& debugUI = m_engine.GetDebugUI();
                const bool visible = !debugUI.IsVisible();
                debugUI.SetVisible(visible);
                m_window.SetCursorCaptured(!visible);
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
// Mouse movement to look around and WASD to move. Does nothing while the debug UI is on.
class CameraControlSystem : public ezse::System {
public:
    CameraControlSystem(ezse::Window& window, ezse::Engine& engine);

protected:
    void Update(ezse::World& world, float deltaTime) override;

private:
    ezse::Window& m_window;
    ezse::Engine& m_engine;
};

CameraControlSystem::CameraControlSystem(ezse::Window& window, ezse::Engine& engine) :
    m_window(window), m_engine(engine) {}

void CameraControlSystem::Update(ezse::World& world, float deltaTime) {
    if (m_engine.GetDebugUI().IsVisible()) {
        return;
    }
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
    // Creating engine
    auto engine = ezse::Engine::Create({.title = "EZSE Sandbox"});
    if (!engine) {
        return 1;
    }

    // Setting references for renderer and resources
    ezse::Renderer& renderer = engine->GetRenderer();
    ezse::ResourceManager& resources = engine->GetResources();

    // Loading shader
    const ezse::ShaderHandle shader = resources.LoadShader(
        "shaders/lit.vert",
        "shaders/lit.frag");
    if (shader.id == 0) {
        return 1;
    }

    // Loading Textures and meshes
    const ezse::TextureHandle checkerTexture = resources.LoadTexture("textures/checker.png");
    const ezse::MeshHandle cubeMesh = resources.LoadMesh("models/cube.obj");
    const ezse::MeshHandle monkeyMesh = resources.LoadMesh("models/monkey.obj");
    const ezse::MeshHandle roomMesh = resources.LoadMesh("models/room.obj");
    if (cubeMesh.id == 0 || monkeyMesh.id == 0 || roomMesh.id == 0 || checkerTexture.id == 0) {
        return 1;
    }
    // Checks the counter of the resource manager: the same file through another path gives the
    // same handle without loading it again, and one Unload does not destroy it yet (2 -> 1).
    const ezse::MeshHandle cubeMeshAgain = resources.LoadMesh("./models/cube.obj");
    ezse::LogInfo("Cube loaded twice: id {} and id {}", cubeMesh.id, cubeMeshAgain.id);
    resources.UnloadMesh(cubeMeshAgain);

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

    // Suzanne from Blender, left of the cubes: half size, with the checkerboard to show her uvs.
    const ezse::Entity monkey = world.CreateEntity();
    world.AddComponent(monkey, ezse::Transform{
        .position = {-1.75f, -2.0f, 0.0f},
        .scale = {0.5f, 0.5f, 0.5f}});
    world.AddComponent(monkey, ezse::MeshRenderer{monkeyMesh, {.shader = shader, .texture = checkerTexture}});
    world.AddComponent(monkey, Spin{.speed = {0.0f, 0.5f, 0.0f}});

    // A room around everything: a floor under the cubes and four walls, open at the top. Grey
    // checkerboard with little shine, so it does not look like the cubes.
    const ezse::Entity room = world.CreateEntity();
    world.AddComponent(room, ezse::Transform{});
    world.AddComponent(room, ezse::MeshRenderer{roomMesh, {
        .shader = shader,
        .texture = checkerTexture,
        .tint = {0.6f, 0.6f, 0.6f},
        .shininess = 8.0f,
        .specularStrength = 0.1f}});

    // The sun: a white directional light, turned so it shines in the direction (-0.5, -1, -0.7),
    // like the light that Sandbox set by hand before.
    const float degrees = std::numbers::pi_v<float> / 180.0f;
    const ezse::Entity sun = world.CreateEntity();
    world.AddComponent(sun, ezse::Transform{.rotation = {-50.6f * degrees, 35.5f * degrees, 0.0f}});
    world.AddComponent(sun, ezse::Light{});
    // The light that reaches every surface from all sides.
    const ezse::Entity ambient = world.CreateEntity();
    world.AddComponent(ambient, ezse::Transform{});
    world.AddComponent(ambient, ezse::Light{
        .type = ezse::LightType::Ambient,
        .color = {0.15f, 0.15f, 0.15f}});
    // A warm lamp just in front of the middle cube (its front face is at z = 0.5).
    const ezse::Entity lamp = world.CreateEntity();
    world.AddComponent(lamp, ezse::Transform{.position = {0.0f, -2.0f, 1.25f}});
    world.AddComponent(lamp, ezse::Light{
        .type = ezse::LightType::Point,
        .color = {1.0f, 0.8f, 0.6f},
        .intensity = 2.0f,
        .range = 8.0f});
    // A cool lamp behind and to the left of Suzanne, so both lamps can be told apart.
    const ezse::Entity blueLamp = world.CreateEntity();
    world.AddComponent(blueLamp, ezse::Transform{.position = {-3.0f, -1.5f, -1.5f}});
    world.AddComponent(blueLamp, ezse::Light{
        .type = ezse::LightType::Point,
        .color = {0.6f, 0.7f, 1.0f},
        .intensity = 2.0f,
        .range = 6.0f});

    // Debug panel for the sun and the ambient light.
    engine->GetDebugUI().AddPanel("Light", [sun, ambient](ezse::World& world) {
        ezse::Vec3& sunRotation = world.GetComponent<ezse::Transform>(sun).rotation;
        ezse::Light& sunLight = world.GetComponent<ezse::Light>(sun);
        ezse::Light& ambientLight = world.GetComponent<ezse::Light>(ambient);
        ImGui::SeparatorText("Sun");
        ImGui::SliderAngle("Pitch", &sunRotation.x, -90.0f, 90.0f);
        ImGui::SliderAngle("Yaw", &sunRotation.y, -180.0f, 180.0f);
        ezse::ColorEditVec3("Color##sun", sunLight.color);
        ImGui::SliderFloat("Intensity##sun", &sunLight.intensity, 0.0f, 2.0f);
        ImGui::SeparatorText("Ambient");
        ezse::ColorEditVec3("Color##ambient", ambientLight.color);
        ImGui::SliderFloat("Intensity##ambient", &ambientLight.intensity, 0.0f, 2.0f);
    });

    // The camera begins at (0, 0, 3) and looks down the negative Z axis.
    const ezse::Entity camera = world.CreateEntity();
    world.AddComponent(camera, ezse::Transform{.position = {0.0f, 0.0f, 3.0f}});
    world.AddComponent(camera, ezse::Camera{});
    world.AddComponent(camera, CameraController{});
    // Making the debug panel for the Camera
    engine->GetDebugUI().AddPanel("Camera", [camera](ezse::World& world) {
        ezse::Vec3& cameraPos = world.GetComponent<ezse::Transform>(camera).position;
        ezse::DragVec3("Position", cameraPos, 0.05f);
    });

    // Registing the systems
    world.AddSystem<GameControlSystem>(ezse::MakeSignature<>(),
        engine->GetWindow(), *engine);
    world.AddSystem<SpinSystem>(ezse::MakeSignature<ezse::Transform, Spin>());
    world.AddSystem<CameraControlSystem>(
        ezse::MakeSignature<ezse::Transform, CameraController>(),
        engine->GetWindow(), *engine);
    // The engine systems come last: camera, then lights, then drawing (decision 115).
    world.AddSystem<ezse::CameraSystem>(ezse::MakeSignature<ezse::Transform, ezse::Camera>(),
                                        engine->GetWindow(), renderer);
    world.AddSystem<ezse::LightSystem>(ezse::MakeSignature<ezse::Transform, ezse::Light>(),
                                       renderer);
    world.AddSystem<ezse::RenderSystem>(ezse::MakeSignature<ezse::Transform, ezse::MeshRenderer>(),
                                        renderer, ezse::Vec3{0.1f, 0.2f, 0.3f});

    // Capturing mouse in window and running game
    engine->GetWindow().SetCursorCaptured(true);
    engine->Run();

    ezse::LogInfo("Sandbox closed");
    return 0;
}
