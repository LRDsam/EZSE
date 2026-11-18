#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/Light.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/PreviousTransform.h"
#include "EZSE/Engine/Components/Transform.h"
#include "EZSE/Engine/Components/WorldMatrix.h"
#include "EZSE/Engine/Engine.h"
#include "EZSE/Engine/Resources/Image.h"
#include "EZSE/Engine/Resources/Obj.h"
#include "EZSE/Engine/Resources/ResourceCache.h"
#include "EZSE/Engine/SimulationTime.h"
#include "EZSE/Engine/Systems/TransformHistorySystem.h"
#include "EZSE/Engine/Systems/WorldMatrixSystem.h"

#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/SystemPhase.h"
#include "EZSE/ECS/World.h"

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"
#include "EZSE/Math/Vec4.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failureCount = 0;

void Check(bool condition, const char* description) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failureCount;
    }
}

bool NearlyEqual(const ezse::Mat4& a, const ezse::Mat4& b) {
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            if (std::abs(a.m[column][row] - b.m[column][row]) > 0.0001f) {
                return false;
            }
        }
    }
    return true;
}

bool NearlyEqual(const ezse::Vec3& a, const ezse::Vec3& b) {
    return std::abs(a.x - b.x) <= 0.0001f && std::abs(a.y - b.y) <= 0.0001f &&
           std::abs(a.z - b.z) <= 0.0001f;
}

bool NearlyEqual(const ezse::Vec4& a, const ezse::Vec4& b) {
    return std::abs(a.x - b.x) <= 0.0001f && std::abs(a.y - b.y) <= 0.0001f &&
           std::abs(a.z - b.z) <= 0.0001f && std::abs(a.w - b.w) <= 0.0001f;
}

// Built by hand so the tests do not depend on the Math functions that ModelMatrix uses.
ezse::Mat4 MakeTranslation(float x, float y, float z) {
    ezse::Mat4 result;
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
}

const float quarterTurn = std::numbers::pi_v<float> / 2.0f;

void TestModelMatrix() {
    Check(NearlyEqual(ezse::ModelMatrix(ezse::Transform{}), ezse::Mat4{}),
          "ModelMatrix: a default Transform gives the identity matrix");

    ezse::Transform moved;
    moved.position = ezse::Vec3{1.0f, 2.0f, 3.0f};
    Check(NearlyEqual(ezse::ModelMatrix(moved), MakeTranslation(1.0f, 2.0f, 3.0f)),
          "ModelMatrix: only a position gives a translation");

    // A quarter turn around Y takes +X to -Z; the translation comes after the rotation.
    ezse::Transform turned;
    turned.position = ezse::Vec3{1.0f, 0.0f, 0.0f};
    turned.rotation = ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn, 0.0f});
    Check(NearlyEqual(ezse::ModelMatrix(turned) * ezse::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
                      ezse::Vec4{1.0f, 0.0f, -1.0f, 1.0f}),
          "ModelMatrix: rotation first, then translation");

    // FromEuler applies X before Y: +X stays on the X axis, then Y turns it to -Z.
    // In the other order the point would end up on +Y.
    ezse::Transform tilted;
    tilted.rotation = ezse::FromEuler(ezse::Vec3{quarterTurn, quarterTurn, 0.0f});
    Check(NearlyEqual(ezse::ModelMatrix(tilted) * ezse::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, -1.0f, 1.0f}),
          "ModelMatrix: the X rotation comes before the Y rotation");

    // Scaling comes first: (1, 0, 0) becomes (2, 0, 0) and then turns to (0, 0, -2).
    ezse::Transform stretched;
    stretched.rotation = ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn, 0.0f});
    stretched.scale = ezse::Vec3{2.0f, 1.0f, 1.0f};
    Check(NearlyEqual(ezse::ModelMatrix(stretched) * ezse::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, -2.0f, 1.0f}),
          "ModelMatrix: scaling comes before the rotation");
}

void TestCameraView() {
    // The CameraSystem draws with the inverse of the WorldMatrix of the camera as its view.
    ezse::Transform camera;
    camera.position = ezse::Vec3{2.0f, 1.0f, 3.0f};
    camera.rotation = ezse::FromEuler(ezse::Vec3{0.3f, -0.7f, 0.2f});
    const ezse::Mat4 view = ezse::Inverse(ezse::ModelMatrix(camera));

    Check(NearlyEqual(view * ezse::ModelMatrix(camera), ezse::Mat4{}),
          "Camera view: the inverse undoes ModelMatrix of the same Transform");

    Check(NearlyEqual(view * ezse::Vec4{2.0f, 1.0f, 3.0f, 1.0f}, ezse::Vec4{0.0f, 0.0f, 0.0f, 1.0f}),
          "Camera view: the camera position ends up in the origin");

    // A camera at (0, 0, 3) turned a quarter to the left looks along -X.
    // A point two units in front of it must end up on the -Z axis of the view.
    ezse::Transform turnedLeft;
    turnedLeft.position = ezse::Vec3{0.0f, 0.0f, 3.0f};
    turnedLeft.rotation = ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn, 0.0f});
    Check(NearlyEqual(ezse::Inverse(ezse::ModelMatrix(turnedLeft)) * ezse::Vec4{-2.0f, 0.0f, 3.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, -2.0f, 1.0f}),
          "Camera view: a point in front of a turned camera lies on -Z");
}

void TestWorldMatrixPositionForward() {
    ezse::Transform transform;
    transform.position = ezse::Vec3{1.0f, -2.0f, 3.0f};
    const ezse::WorldMatrix moved{ezse::ModelMatrix(transform)};
    Check(NearlyEqual(ezse::Position(moved), ezse::Vec3{1.0f, -2.0f, 3.0f}),
          "WorldMatrix Position: the translation of the matrix");
    Check(NearlyEqual(ezse::Forward(moved), ezse::Vec3{0.0f, 0.0f, -1.0f}),
          "WorldMatrix Forward: an entity that is not turned looks along -Z, wherever it stands");

    // A quarter to the left around Y: -Z turns to -X.
    transform.rotation = ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn, 0.0f});
    const ezse::WorldMatrix turned{ezse::ModelMatrix(transform)};
    Check(NearlyEqual(ezse::Forward(turned), ezse::Vec3{-1.0f, 0.0f, 0.0f}),
          "WorldMatrix Forward: a quarter turn left around Y looks along -X");
    Check(NearlyEqual(ezse::Position(turned), ezse::Vec3{1.0f, -2.0f, 3.0f}),
          "WorldMatrix Position: a rotation does not move the position");

    transform.scale = ezse::Vec3{2.0f, 3.0f, 4.0f};
    const ezse::WorldMatrix scaled{ezse::ModelMatrix(transform)};
    Check(NearlyEqual(ezse::Forward(scaled), ezse::Vec3{-1.0f, 0.0f, 0.0f}),
          "WorldMatrix Forward: of length 1, also with a scale");
}

void TestComponentDefaults() {
    const ezse::Camera camera;
    Check(std::abs(camera.fovY - std::numbers::pi_v<float> / 4.0f) < 0.0001f,
          "Camera: the default field of view is 45 degrees");
    Check(camera.nearPlane < camera.farPlane, "Camera: the near plane lies before the far plane");

    const ezse::MeshRenderer meshRenderer;
    Check(meshRenderer.mesh.id == 0 && meshRenderer.material.shader.id == 0 &&
              meshRenderer.material.texture.id == 0,
          "MeshRenderer: a default MeshRenderer has no mesh, shader or texture");

    // The defaults are the constants lit.frag had before materials (decision 177).
    const ezse::Material material;
    Check(material.tint.x == 1.0f && material.tint.y == 1.0f && material.tint.z == 1.0f,
          "Material: the default tint is white");
    Check(material.shininess == 128.0f && material.specularStrength == 0.5f,
          "Material: the default shine is shininess 128 and strength 0.5");

    const ezse::Light light;
    Check(light.type == ezse::LightType::Directional, "Light: a default light is directional");
    Check(light.color.x == 1.0f && light.color.y == 1.0f && light.color.z == 1.0f &&
              light.intensity == 1.0f,
          "Light: the default light is white with intensity 1");
    Check(light.range > 0.0f, "Light: the default range is larger than 0");
    Check(light.innerAngle > 0.0f && light.innerAngle < light.outerAngle,
          "Light: the inner spot angle lies inside the outer one");
}

// A PNG of 2 by 2 pixels, made by a script. In the file the top row is red, green and the
// bottom row is blue, white (all fully opaque).
const std::uint8_t c_testPng[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x72, 0xB6, 0x0D, 0x24, 0x00, 0x00, 0x00,
    0x12, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0x63, 0xF8, 0xCF, 0xC0, 0xF0,
    0x1F, 0x0C, 0x81, 0x34, 0x18, 0x00, 0x00, 0x49, 0xC8, 0x09, 0xF7, 0x03,
    0xD9, 0x64, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
    0x42, 0x60, 0x82,
};

// The 4 bytes of the pixel at (column, row) of an Image; row 0 is the first row in memory.
bool PixelIs(const ezse::Image& image, int column, int row,
             std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    const std::size_t index = (static_cast<std::size_t>(row) * static_cast<std::size_t>(image.width) +
                               static_cast<std::size_t>(column)) * 4;
    return image.pixels[index] == red && image.pixels[index + 1] == green &&
           image.pixels[index + 2] == blue && image.pixels[index + 3] == alpha;
}

void TestDecodeImage() {
    const std::optional<ezse::Image> image = ezse::DecodeImage(c_testPng);
    Check(image.has_value(), "DecodeImage: a valid PNG decodes");
    if (!image) {
        return;
    }

    Check(image->width == 2 && image->height == 2, "DecodeImage: width and height are 2");
    Check(image->pixels.size() == 16, "DecodeImage: 4 bytes per pixel");
    // Flipped: the bottom row of the file comes first in memory.
    Check(PixelIs(*image, 0, 0, 0, 0, 255, 255) && PixelIs(*image, 1, 0, 255, 255, 255, 255),
          "DecodeImage: the first row in memory is the bottom row of the file (blue, white)");
    Check(PixelIs(*image, 0, 1, 255, 0, 0, 255) && PixelIs(*image, 1, 1, 0, 255, 0, 255),
          "DecodeImage: the second row in memory is the top row of the file (red, green)");

    const std::uint8_t notAnImage[] = {1, 2, 3, 4, 5, 6, 7, 8};
    Check(!ezse::DecodeImage(notAnImage).has_value(), "DecodeImage: bytes that are not an image give nothing");
}

// The positions, uvs and normal of a square in the XY plane, facing +Z. A test adds the faces.
constexpr std::string_view c_squareData =
    "v -0.5 -0.5 0.0\n"
    "v  0.5 -0.5 0.0\n"
    "v  0.5  0.5 0.0\n"
    "v -0.5  0.5 0.0\n"
    "vt 0 0\n"
    "vt 1 0\n"
    "vt 1 1\n"
    "vt 0 1\n"
    "vn 0 0 1\n";

bool IndicesAre(const ezse::MeshData& mesh, const std::vector<std::uint32_t>& expected) {
    return mesh.indices == expected;
}

void TestParseObj() {
    // Two triangles share two corners: those become one Vertex each.
    const std::string twoTriangles = std::string(c_squareData) +
        "f 1/1/1 2/2/1 3/3/1\n"
        "f 1/1/1 3/3/1 4/4/1\n";
    const std::optional<ezse::MeshData> square = ezse::ParseObj(twoTriangles);
    Check(square.has_value(), "ParseObj: a square of two triangles parses");
    if (square) {
        Check(square->vertices.size() == 4 && square->indices.size() == 6,
              "ParseObj: shared corners are merged (4 vertices, 6 indices)");
        Check(IndicesAre(*square, {0, 1, 2, 0, 2, 3}), "ParseObj: the indices follow the faces");
        const ezse::Vertex& corner = square->vertices[2];
        Check(corner.position[0] == 0.5f && corner.position[1] == 0.5f && corner.position[2] == 0.0f &&
                  corner.normal[2] == 1.0f && corner.uv[0] == 1.0f && corner.uv[1] == 1.0f,
              "ParseObj: a vertex gets its position, normal and uv");
    }

    // One face with four corners becomes a fan of two triangles.
    const std::optional<ezse::MeshData> quad =
        ezse::ParseObj(std::string(c_squareData) + "f 1/1/1 2/2/1 3/3/1 4/4/1\n");
    Check(quad.has_value() && quad->vertices.size() == 4 && IndicesAre(*quad, {0, 1, 2, 0, 2, 3}),
          "ParseObj: a face with four corners becomes two triangles");

    // Windows line endings, extra spaces, comments and other keywords are fine.
    const std::optional<ezse::MeshData> windows = ezse::ParseObj(
        "# a comment\r\no square\r\nv -0.5 -0.5 0.0\r\nv  0.5  -0.5   0.0\r\nv 0.5 0.5 0.0\r\n"
        "vt 0 0\r\nvn 0 0 1\r\ns off\r\nusemtl none\r\n\r\nf 1/1/1 2/1/1 3/1/1");
    Check(windows.has_value() && windows->indices.size() == 3,
          "ParseObj: CRLF, extra spaces, comments and other keywords are accepted");

    std::cout << "(the next four checks log an error on purpose)\n";
    Check(!ezse::ParseObj(std::string(c_squareData) + "f 1/1/1 2/2/1 5/3/1\n").has_value(),
          "ParseObj: an index past what was read gives nothing");
    Check(!ezse::ParseObj(std::string(c_squareData) + "f 1//1 2//1 3//1\n").has_value(),
          "ParseObj: a corner without uv gives nothing");
    Check(!ezse::ParseObj("v 1.0 O.5 3.0\n").has_value(),
          "ParseObj: a number with a letter in it gives nothing");
    Check(!ezse::ParseObj(c_squareData).has_value(), "ParseObj: text without faces gives nothing");
}

// The handles are made up here: the cache only stores them and never asks the Renderer.
void TestResourceCache() {
    ezse::ResourceCache<ezse::MeshHandle> cache;
    Check(cache.Acquire("models/cube.obj").id == 0, "ResourceCache: an unknown key gives id 0");
    Check(cache.GetHandles().empty(), "ResourceCache: a new cache has no handles");

    const ezse::MeshHandle cube{7};
    cache.Add("models/cube.obj", cube);
    Check(cache.GetKey(cube) == "models/cube.obj", "ResourceCache: GetKey gives the key of a handle");
    Check(cache.Acquire("models/cube.obj").id == 7, "ResourceCache: a known key gives its handle");

    const ezse::MeshHandle monkey{9};
    cache.Add("models/monkey.obj", monkey);
    Check(cache.GetHandles().size() == 2, "ResourceCache: GetHandles gives every handle");

    // The cube is at count 2 now (Add, then one Acquire).
    Check(!cache.Release(cube), "ResourceCache: the first Release of two is not the last");
    Check(cache.Acquire("models/cube.obj").id == 7, "ResourceCache: after one Release the key is still known");
    Check(!cache.Release(cube), "ResourceCache: Release counts down an Acquire");
    Check(cache.Release(cube), "ResourceCache: the last Release returns true");
    Check(cache.Acquire("models/cube.obj").id == 0, "ResourceCache: after the last Release the key is unknown");

    const std::vector<ezse::MeshHandle> left = cache.GetHandles();
    Check(left.size() == 1 && left[0].id == 9, "ResourceCache: other keys keep their own count");
    Check(cache.Release(monkey), "ResourceCache: a single Add needs a single Release");
    Check(cache.GetHandles().empty(), "ResourceCache: the cache is empty after everything is released");

    // A key can be added again after its last Release, with a new handle.
    cache.Add("models/cube.obj", ezse::MeshHandle{12});
    Check(cache.Acquire("models/cube.obj").id == 12, "ResourceCache: a released key can be added again");
}

// Exact comparison: the system and ResetInterpolation copy, they do not compute.
bool SameTransform(const ezse::Transform& a, const ezse::Transform& b) {
    return a.position.x == b.position.x && a.position.y == b.position.y &&
           a.position.z == b.position.z && a.rotation.x == b.rotation.x &&
           a.rotation.y == b.rotation.y && a.rotation.z == b.rotation.z &&
           a.rotation.w == b.rotation.w && a.scale.x == b.scale.x && a.scale.y == b.scale.y &&
           a.scale.z == b.scale.z;
}

// The Engine needs a window, so the test builds the same World setup as the Engine constructor:
// both components and the TransformHistorySystem in SystemPhase::Simulation.
void TestInterpolationHistory() {
    ezse::World world;
    world.RegisterComponent<ezse::Transform>();
    world.RegisterComponent<ezse::PreviousTransform>();
    world.AddSystem<ezse::TransformHistorySystem>(
        ezse::SimulationGroup::PreUpdate,
        ezse::MakeSignature<ezse::Transform, ezse::PreviousTransform>());

    const ezse::Entity interpolated = world.CreateEntity();
    world.AddComponent(interpolated, ezse::Transform{.position = {1.0f, 2.0f, 3.0f}});
    ezse::EnableInterpolation(world, interpolated);
    Check(world.HasComponent<ezse::PreviousTransform>(interpolated),
          "EnableInterpolation: adds a PreviousTransform");
    Check(SameTransform(world.GetComponent<ezse::PreviousTransform>(interpolated).transform,
                        world.GetComponent<ezse::Transform>(interpolated)),
          "EnableInterpolation: the previous state equals the current one");

    const ezse::Entity plain = world.CreateEntity();
    world.AddComponent(plain, ezse::Transform{});

    // As a game system would do during a tick: move, turn and scale.
    const ezse::Transform start = world.GetComponent<ezse::Transform>(interpolated);
    const ezse::Transform moved{.position = {4.0f, 5.0f, 6.0f},
                                .rotation = ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn, 0.0f}),
                                .scale = {2.0f, 2.0f, 2.0f}};
    world.GetComponent<ezse::Transform>(interpolated) = moved;
    Check(SameTransform(world.GetComponent<ezse::PreviousTransform>(interpolated).transform, start),
          "PreviousTransform: a change of the Transform does not change it by itself");

    world.Update(ezse::SystemPhase::Presentation, 1.0f / 60.0f);
    Check(SameTransform(world.GetComponent<ezse::PreviousTransform>(interpolated).transform, start),
          "TransformHistorySystem: does not copy in Presentation");

    world.Update(ezse::SystemPhase::Simulation, 1.0f / 30.0f);
    Check(SameTransform(world.GetComponent<ezse::PreviousTransform>(interpolated).transform, moved),
          "TransformHistorySystem: a tick copies position, rotation and scale");
    Check(!world.HasComponent<ezse::PreviousTransform>(plain),
          "TransformHistorySystem: an entity without interpolation gets no PreviousTransform");

    // A teleport after the copy of this tick: without a reset the next frames would blend.
    const ezse::Transform teleported{.position = {100.0f, 0.0f, 0.0f}};
    world.GetComponent<ezse::Transform>(interpolated) = teleported;
    ezse::ResetInterpolation(world, interpolated);
    Check(SameTransform(world.GetComponent<ezse::PreviousTransform>(interpolated).transform,
                        teleported),
          "ResetInterpolation: the previous state equals the current one");

    ezse::ResetInterpolation(world, plain);
    Check(!world.HasComponent<ezse::PreviousTransform>(plain),
          "ResetInterpolation: does nothing for an entity without interpolation");
}

bool NearlyEqual(const ezse::Quat& a, const ezse::Quat& b) {
    return std::abs(a.x - b.x) <= 0.0001f && std::abs(a.y - b.y) <= 0.0001f &&
           std::abs(a.z - b.z) <= 0.0001f && std::abs(a.w - b.w) <= 0.0001f;
}

bool NearlyEqual(const ezse::Transform& a, const ezse::Transform& b) {
    return NearlyEqual(a.position, b.position) && NearlyEqual(a.rotation, b.rotation) &&
           NearlyEqual(a.scale, b.scale);
}

void TestInterpolate() {
    const ezse::Transform a{.position = {0.0f, 0.0f, 0.0f}, .scale = {1.0f, 1.0f, 1.0f}};
    const ezse::Transform b{.position = {2.0f, 4.0f, -6.0f},
                            .rotation = ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn, 0.0f}),
                            .scale = {3.0f, 3.0f, 3.0f}};

    Check(NearlyEqual(ezse::Interpolate(a, b, 0.0f), a), "Interpolate: t = 0 gives a");
    Check(NearlyEqual(ezse::Interpolate(a, b, 1.0f), b), "Interpolate: t = 1 gives b");

    const ezse::Transform half = ezse::Interpolate(a, b, 0.5f);
    Check(NearlyEqual(half.position, ezse::Vec3{1.0f, 2.0f, -3.0f}),
          "Interpolate: t = 0.5 puts the position halfway");
    Check(NearlyEqual(half.scale, ezse::Vec3{2.0f, 2.0f, 2.0f}),
          "Interpolate: t = 0.5 puts the scale halfway");
    // Halfway along a quarter turn around Y is an eighth turn around Y.
    Check(NearlyEqual(half.rotation, ezse::FromEuler(ezse::Vec3{0.0f, quarterTurn / 2.0f, 0.0f})),
          "Interpolate: t = 0.5 turns the rotation halfway (Slerp)");
}

// The same World setup as the Engine constructor, with a SimulationTime of its own whose alpha the
// test sets by hand.
void TestWorldMatrixSystem() {
    ezse::World world;
    world.RegisterComponent<ezse::Transform>();
    world.RegisterComponent<ezse::PreviousTransform>();
    world.RegisterComponent<ezse::WorldMatrix>();
    ezse::SimulationTime time;
    world.AddSystem<ezse::WorldMatrixSystem>(
        ezse::PresentationGroup::PostUpdate,
        ezse::MakeSignature<ezse::Transform>(),
        time);

    const ezse::Entity plain = world.CreateEntity();
    world.AddComponent(plain, ezse::Transform{.position = {1.0f, 2.0f, 3.0f}});

    const ezse::Entity interpolated = world.CreateEntity();
    world.AddComponent(interpolated, ezse::Transform{});
    ezse::EnableInterpolation(world, interpolated);
    // As after a tick: the previous state is at the origin, the current one 2 along X.
    world.GetComponent<ezse::Transform>(interpolated).position = ezse::Vec3{2.0f, 0.0f, 0.0f};

    const ezse::Entity withoutTransform = world.CreateEntity();

    time.alpha = 0.25f;
    world.Update(ezse::SystemPhase::Presentation, 1.0f / 60.0f);
    Check(world.HasComponent<ezse::WorldMatrix>(plain) &&
              world.HasComponent<ezse::WorldMatrix>(interpolated),
          "WorldMatrixSystem: adds a WorldMatrix to every entity with a Transform");
    Check(!world.HasComponent<ezse::WorldMatrix>(withoutTransform),
          "WorldMatrixSystem: an entity without a Transform gets no WorldMatrix");
    Check(NearlyEqual(world.GetComponent<ezse::WorldMatrix>(plain).matrix,
                      MakeTranslation(1.0f, 2.0f, 3.0f)),
          "WorldMatrixSystem: without interpolation the matrix is the ModelMatrix of the Transform");
    Check(NearlyEqual(world.GetComponent<ezse::WorldMatrix>(interpolated).matrix,
                      MakeTranslation(0.5f, 0.0f, 0.0f)),
          "WorldMatrixSystem: with interpolation the matrix is blended by alpha");

    // The system holds a reference: a new alpha is seen without adding the system again.
    time.alpha = 0.75f;
    world.Update(ezse::SystemPhase::Presentation, 1.0f / 60.0f);
    Check(NearlyEqual(world.GetComponent<ezse::WorldMatrix>(interpolated).matrix,
                      MakeTranslation(1.5f, 0.0f, 0.0f)),
          "WorldMatrixSystem: reads the current alpha of the SimulationTime every frame");

    world.GetComponent<ezse::Transform>(plain).position = ezse::Vec3{5.0f, 0.0f, 0.0f};
    world.Update(ezse::SystemPhase::Simulation, 1.0f / 30.0f);
    Check(NearlyEqual(world.GetComponent<ezse::WorldMatrix>(plain).matrix,
                      MakeTranslation(1.0f, 2.0f, 3.0f)),
          "WorldMatrixSystem: does not run in Simulation");
}

void TestHeadlessEngine() {
    // The first Engine in the tests: without a window it needs no GPU.
    std::unique_ptr<ezse::Engine> engine = ezse::Engine::Create({.window = std::nullopt});
    Check(engine != nullptr, "Headless Engine: Create succeeds without a window");
    if (!engine) {
        return;
    }
    Check(engine->IsHeadless(), "Headless Engine: IsHeadless is true");
    Check(engine->GetSimulationTime().tick == 0, "Headless Engine: starts at tick 0");

    // The built-in components are registered, also without a window.
    ezse::World& world = engine->GetWorld();
    const ezse::Entity entity = world.CreateEntity();
    world.AddComponent(entity, ezse::Transform{});
    world.AddComponent(entity, ezse::WorldMatrix{});
    Check(world.HasComponent<ezse::Transform>(entity) && world.HasComponent<ezse::WorldMatrix>(entity),
          "Headless Engine: the built-in components are registered");

    Check(ezse::Engine::Create({.window = std::nullopt, .ticksPerSecond = 0}) == nullptr,
          "Headless Engine: ticksPerSecond 0 is still refused");
}

} // namespace

int main() {
    TestModelMatrix();
    TestCameraView();
    TestWorldMatrixPositionForward();
    TestComponentDefaults();
    TestDecodeImage();
    TestParseObj();
    TestResourceCache();
    TestInterpolationHistory();
    TestInterpolate();
    TestWorldMatrixSystem();
    TestHeadlessEngine();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
