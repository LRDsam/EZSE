#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"
#include "EZSE/Math/Vec4.h"

#include <cmath>
#include <iostream>
#include <numbers>

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
    turned.rotation.y = quarterTurn;
    Check(NearlyEqual(ezse::ModelMatrix(turned) * ezse::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
                      ezse::Vec4{1.0f, 0.0f, -1.0f, 1.0f}),
          "ModelMatrix: rotation first, then translation");

    // X is applied before Y: +X stays on the X axis, then Y turns it to -Z.
    // In the other order the point would end up on +Y.
    ezse::Transform tilted;
    tilted.rotation.x = quarterTurn;
    tilted.rotation.y = quarterTurn;
    Check(NearlyEqual(ezse::ModelMatrix(tilted) * ezse::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, -1.0f, 1.0f}),
          "ModelMatrix: the X rotation comes before the Y rotation");

    // Scaling comes first: (1, 0, 0) becomes (2, 0, 0) and then turns to (0, 0, -2).
    ezse::Transform stretched;
    stretched.rotation.y = quarterTurn;
    stretched.scale = ezse::Vec3{2.0f, 1.0f, 1.0f};
    Check(NearlyEqual(ezse::ModelMatrix(stretched) * ezse::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, -2.0f, 1.0f}),
          "ModelMatrix: scaling comes before the rotation");
}

void TestViewMatrix() {
    ezse::Transform camera;
    camera.position = ezse::Vec3{2.0f, 1.0f, 3.0f};
    camera.rotation = ezse::Vec3{0.3f, -0.7f, 0.2f};

    Check(NearlyEqual(ezse::ViewMatrix(camera) * ezse::ModelMatrix(camera), ezse::Mat4{}),
          "ViewMatrix: undoes ModelMatrix of the same Transform (scale 1)");

    Check(NearlyEqual(ezse::ViewMatrix(camera) * ezse::Vec4{2.0f, 1.0f, 3.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, 0.0f, 1.0f}),
          "ViewMatrix: the camera position ends up in the origin");

    // A camera at (0, 0, 3) turned a quarter to the left looks along -X.
    // A point two units in front of it must end up on the -Z axis of the view.
    ezse::Transform turnedLeft;
    turnedLeft.position = ezse::Vec3{0.0f, 0.0f, 3.0f};
    turnedLeft.rotation.y = quarterTurn;
    Check(NearlyEqual(ezse::ViewMatrix(turnedLeft) * ezse::Vec4{-2.0f, 0.0f, 3.0f, 1.0f},
                      ezse::Vec4{0.0f, 0.0f, -2.0f, 1.0f}),
          "ViewMatrix: a point in front of a turned camera lies on -Z");
}

void TestComponentDefaults() {
    const ezse::Camera camera;
    Check(std::abs(camera.fovY - std::numbers::pi_v<float> / 4.0f) < 0.0001f,
          "Camera: the default field of view is 45 degrees");
    Check(camera.nearPlane < camera.farPlane, "Camera: the near plane lies before the far plane");

    const ezse::MeshRenderer meshRenderer;
    Check(meshRenderer.mesh.id == 0 && meshRenderer.shader.id == 0,
          "MeshRenderer: a default MeshRenderer has no mesh and no shader");
}

} // namespace

int main() {
    TestModelMatrix();
    TestViewMatrix();
    TestComponentDefaults();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
