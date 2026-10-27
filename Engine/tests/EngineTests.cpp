#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"
#include "EZSE/Engine/Resources/Image.h"

#include "EZSE/Math/Mat4.h"
#include "EZSE/Math/Vec3.h"
#include "EZSE/Math/Vec4.h"

#include <cmath>
#include <cstdint>
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
    Check(meshRenderer.mesh.id == 0 && meshRenderer.shader.id == 0 && meshRenderer.texture.id == 0,
          "MeshRenderer: a default MeshRenderer has no mesh, shader or texture");
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

} // namespace

int main() {
    TestModelMatrix();
    TestViewMatrix();
    TestComponentDefaults();
    TestDecodeImage();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
