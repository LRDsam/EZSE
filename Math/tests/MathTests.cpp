#include "EZSE/Math/Transform.h"

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

bool NearlyEqual(const ezse::Vec3& a, const ezse::Vec3& b) {
    return std::abs(a.x - b.x) <= 0.0001f && std::abs(a.y - b.y) <= 0.0001f &&
           std::abs(a.z - b.z) <= 0.0001f;
}

bool NearlyEqual(const ezse::Vec4& a, const ezse::Vec4& b) {
    return std::abs(a.x - b.x) <= 0.0001f && std::abs(a.y - b.y) <= 0.0001f &&
           std::abs(a.z - b.z) <= 0.0001f && std::abs(a.w - b.w) <= 0.0001f;
}

// What the graphics card does after the vertex shader: divide by w.
ezse::Vec4 PerspectiveDivide(const ezse::Vec4& clip) {
    return ezse::Vec4{ clip.x / clip.w, clip.y / clip.w, clip.z / clip.w, 1.0f };
}

// Built by hand so the tests do not depend on the Math 
// functions they are checking.
ezse::Mat4 MakeTranslation(float x, float y, float z) {
    ezse::Mat4 result;
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
}

ezse::Mat4 MakeScale(const ezse::Vec3& factor) {
    ezse::Mat4 result;
    result.m[0][0] = factor.x;
    result.m[1][1] = factor.y;
    result.m[2][2] = factor.z;
    return result;
}

ezse::Mat4 MakeScale(float factor) {
    return MakeScale(ezse::Vec3{ factor, factor, factor });
}

void TestDefaultValue() {
    const ezse::Mat4 identity;

    Check(identity.m[0][0] == 1.0f && identity.m[1][1] == 1.0f &&
          identity.m[2][2] == 1.0f && identity.m[3][3] == 1.0f,
          "a default Mat4 has ones on the diagonal");
    Check(identity.m[3][0] == 0.0f && identity.m[0][3] == 0.0f,
          "a default Mat4 has zeros off the diagonal");
    Check(NearlyEqual(identity, ezse::Mat4{}), 
        "two default Mat4s are nearly equal");
}

void TestVec3Add() {
    const ezse::Vec3 first{ 1.0f, 2.0f, 3.0f };
    const ezse::Vec3 second{ 4.0f, 5.0f, 6.0f };

    Check(NearlyEqual(first + second, ezse::Vec3{ 5.0f, 7.0f, 9.0f }),
        "Vec3 + Vec3 adds each coordinate");
    Check(NearlyEqual(first + ezse::Vec3{}, first),
        "adding a zero Vec3 changes nothing");
    Check(NearlyEqual(first + second, second + first),
        "the order of a Vec3 addition does not matter");
}

void TestVec3Negate() {
    const ezse::Vec3 vec{ 1.0f, -2.0f, 3.0f };

    Check(NearlyEqual(-vec, ezse::Vec3{ -1.0f, 2.0f, -3.0f }),
        "-Vec3 flips the sign of each coordinate");
    Check(NearlyEqual(-(-vec), vec),
        "negating a Vec3 twice gives the original");
    Check(NearlyEqual(vec + (-vec), ezse::Vec3{}),
        "a Vec3 plus its negation is zero");
}

void TestVec3Scale() {
    const ezse::Vec3 vec{ 1.0f, -2.0f, 3.0f };

    Check(NearlyEqual(vec * 2.0f, ezse::Vec3{ 2.0f, -4.0f, 6.0f }),
        "Vec3 * 2 doubles each coordinate");
    Check(NearlyEqual(vec * 0.0f, ezse::Vec3{}),
        "Vec3 * 0 is zero");
    Check(NearlyEqual(vec * -1.0f, -vec),
        "Vec3 * -1 is the same as negating");
    Check(NearlyEqual(vec * 0.5f, ezse::Vec3{ 0.5f, -1.0f, 1.5f }),
        "Vec3 * 0.5 halves each coordinate");
    Check(NearlyEqual(vec, ezse::Vec3{ 1.0f, -2.0f, 3.0f }),
        "Vec3 * float leaves the original Vec3 unchanged");
}

void TestMultiply() {
    const ezse::Mat4 identity;
    const ezse::Mat4 distinct{{
        { 1.0f,  2.0f,  3.0f,  4.0f},
        { 5.0f,  6.0f,  7.0f,  8.0f},
        { 9.0f, 10.0f, 11.0f, 12.0f},
        {13.0f, 14.0f, 15.0f, 16.0f},
    }};

    Check(NearlyEqual(identity * identity, identity), 
        "identity * identity is the identity");
    Check(NearlyEqual(distinct * identity, distinct), 
        "A * identity is A");
    Check(NearlyEqual(identity * distinct, distinct), 
        "identity * A is A");

    Check(NearlyEqual(
        MakeTranslation(4.0f, 5.0f, 6.0f) * MakeTranslation(1.0f, 2.0f, 3.0f),
                      MakeTranslation(5.0f, 7.0f, 9.0f)),
          "two translations add up");

    // A * B applies B first. 
    // The two orders below only differ in the translation.
    const ezse::Mat4 translation = MakeTranslation(1.0f, 0.0f, 0.0f);
    const ezse::Mat4 scale = MakeScale(2.0f);

    ezse::Mat4 scaleThenTranslate = MakeScale(2.0f);
    scaleThenTranslate.m[3][0] = 1.0f;
    Check(NearlyEqual(translation * scale, scaleThenTranslate),
          "T * S scales first, so the translation stays 1");

    ezse::Mat4 translateThenScale = MakeScale(2.0f);
    translateThenScale.m[3][0] = 2.0f;
    Check(NearlyEqual(scale * translation, translateThenScale),
          "S * T translates first, so the translation is scaled to 2");
}

void TestTranslation() {
    const ezse::Vec3 first{1.0f, 2.0f, 3.0f};
    const ezse::Vec3 second{4.0f, 5.0f, 6.0f};

    Check(NearlyEqual(ezse::Translation(first), MakeTranslation(1.0f, 2.0f, 3.0f)),
          "Translation puts the offset in the last column");
    Check(NearlyEqual(ezse::Translation(ezse::Vec3{}), ezse::Mat4{}),
          "Translation by a zero offset is the identity");
    Check(NearlyEqual(ezse::Translation(first) * ezse::Translation(second),
                      MakeTranslation(5.0f, 7.0f, 9.0f)),
          "two Translations multiply into the sum of their offsets");
}

void TestScale() {
    const ezse::Vec3 first{ 2.0f, 3.0f, 4.0f };
    Check(NearlyEqual(ezse::Scale(first), MakeScale(first)),
        "Scale puts the factors on the diagonal");
    Check(NearlyEqual(ezse::Scale(ezse::Vec3{ 1.0f, 1.0f, 1.0f }), ezse::Mat4{}),
        "Scale by factors of one is the identity");

    const ezse::Vec3 second{ 5.0f, 6.0f, 7.0f };
    Check(NearlyEqual(ezse::Scale(first) * ezse::Scale(second),
                      MakeScale(ezse::Vec3{ 10.0f, 18.0f, 28.0f })),
        "two Scales multiply into the product of their factors");
}

// A quarter turn only contains zeros, ones and a minus one, so the expected
// matrices below are filled in by hand, without sin or cos.
const float c_quarterTurn = std::numbers::pi_v<float> / 2.0f;

void TestRotationX() {
    // The Y axis ends up on Z, the Z axis on minus Y.
    ezse::Mat4 expected;
    expected.m[1][1] = 0.0f;
    expected.m[1][2] = 1.0f;
    expected.m[2][1] = -1.0f;
    expected.m[2][2] = 0.0f;

    Check(NearlyEqual(ezse::RotationX(0.0f), ezse::Mat4{}),
        "RotationX by zero is the identity");
    Check(NearlyEqual(ezse::RotationX(c_quarterTurn), expected),
        "RotationX by a quarter turn moves Y onto Z");
    Check(NearlyEqual(ezse::RotationX(0.3f) * ezse::RotationX(0.5f), ezse::RotationX(0.8f)),
        "two RotationX multiply into the sum of their angles");
    Check(NearlyEqual(ezse::RotationX(0.7f) * ezse::RotationX(-0.7f), ezse::Mat4{}),
        "RotationX by an angle and its opposite is the identity");
}

void TestRotationY() {
    // The Z axis ends up on X, the X axis on minus Z.
    ezse::Mat4 expected;
    expected.m[2][2] = 0.0f;
    expected.m[2][0] = 1.0f;
    expected.m[0][2] = -1.0f;
    expected.m[0][0] = 0.0f;

    Check(NearlyEqual(ezse::RotationY(0.0f), ezse::Mat4{}),
        "RotationY by zero is the identity");
    Check(NearlyEqual(ezse::RotationY(c_quarterTurn), expected),
        "RotationY by a quarter turn moves Z onto X");
    Check(NearlyEqual(ezse::RotationY(0.3f) * ezse::RotationY(0.5f), ezse::RotationY(0.8f)),
        "two RotationY multiply into the sum of their angles");
    Check(NearlyEqual(ezse::RotationY(0.7f) * ezse::RotationY(-0.7f), ezse::Mat4{}),
        "RotationY by an angle and its opposite is the identity");
}

void TestRotationZ() {
    // The X axis ends up on Y, the Y axis on minus X.
    ezse::Mat4 expected;
    expected.m[0][0] = 0.0f;
    expected.m[0][1] = 1.0f;
    expected.m[1][0] = -1.0f;
    expected.m[1][1] = 0.0f;

    Check(NearlyEqual(ezse::RotationZ(0.0f), ezse::Mat4{}),
        "RotationZ by zero is the identity");
    Check(NearlyEqual(ezse::RotationZ(c_quarterTurn), expected),
        "RotationZ by a quarter turn moves X onto Y");
    Check(NearlyEqual(ezse::RotationZ(0.3f) * ezse::RotationZ(0.5f), ezse::RotationZ(0.8f)),
        "two RotationZ multiply into the sum of their angles");
    Check(NearlyEqual(ezse::RotationZ(0.7f) * ezse::RotationZ(-0.7f), ezse::Mat4{}),
        "RotationZ by an angle and its opposite is the identity");
}

void TestTransformDirection() {
    const ezse::Vec3 forward{ 0.0f, 0.0f, -1.0f };
    const float quarterTurn = std::numbers::pi_v<float> / 2.0f;

    Check(NearlyEqual(ezse::TransformDirection(ezse::Mat4{}, forward), forward),
        "TransformDirection: the identity matrix leaves a direction unchanged");
    Check(NearlyEqual(ezse::TransformDirection(MakeTranslation(5.0f, 6.0f, 7.0f), forward), forward),
        "TransformDirection: a translation does not move a direction");
    // A quarter turn to the left around Y takes forward (-Z) to -X.
    Check(NearlyEqual(ezse::TransformDirection(ezse::RotationY(quarterTurn), forward),
                      ezse::Vec3{ -1.0f, 0.0f, 0.0f }),
        "TransformDirection: a quarter turn around Y takes -Z to -X");
    Check(NearlyEqual(ezse::TransformDirection(MakeScale(ezse::Vec3{ 2.0f, 2.0f, 2.0f }), forward),
                      ezse::Vec3{ 0.0f, 0.0f, -2.0f }),
        "TransformDirection: a scale changes the length of a direction");
    Check(NearlyEqual(ezse::TransformDirection(MakeTranslation(1.0f, 0.0f, 0.0f) * ezse::RotationY(quarterTurn), forward),
                      ezse::Vec3{ -1.0f, 0.0f, 0.0f }),
        "TransformDirection: rotation and translation together only rotate");
}

void TestRotationOrder() {
    Check(!NearlyEqual(ezse::RotationX(c_quarterTurn) * ezse::RotationY(c_quarterTurn),
                       ezse::RotationY(c_quarterTurn) * ezse::RotationX(c_quarterTurn)),
        "RotationX * RotationY differs from RotationY * RotationX");
}

void TestTransformVector() {
    const ezse::Vec4 point{ 4.0f, 5.0f, 6.0f, 1.0f };
    const ezse::Vec4 direction{ 4.0f, 5.0f, 6.0f, 0.0f };
    const ezse::Mat4 translation = MakeTranslation(1.0f, 2.0f, 3.0f);

    Check(NearlyEqual(ezse::Mat4{} * point, point),
        "the identity leaves a vector unchanged");
    Check(NearlyEqual(translation * point, ezse::Vec4{ 5.0f, 7.0f, 9.0f, 1.0f }),
        "a translation moves a point (w = 1)");
    Check(NearlyEqual(translation * direction, direction),
        "a translation leaves a direction alone (w = 0)");
    Check(NearlyEqual(MakeScale(ezse::Vec3{ 2.0f, 3.0f, 4.0f }) * ezse::Vec4{ 1.0f, 1.0f, 1.0f, 1.0f },
                      ezse::Vec4{ 2.0f, 3.0f, 4.0f, 1.0f }),
        "a scale multiplies each coordinate by its factor");
    Check(NearlyEqual(ezse::RotationZ(c_quarterTurn) * ezse::Vec4{ 1.0f, 0.0f, 0.0f, 1.0f },
                      ezse::Vec4{ 0.0f, 1.0f, 0.0f, 1.0f }),
        "RotationZ by a quarter turn moves the point (1, 0, 0) to (0, 1, 0)");
}

void TestPerspective() {
    const float fovY = std::numbers::pi_v<float> / 3.0f;
    const float aspect = 2.0f;
    const float nearPlane = 0.5f;
    const float farPlane = 10.0f;
    const ezse::Mat4 projection = ezse::Perspective(fovY, aspect, nearPlane, farPlane);

    // Half the height and width of the visible area at the near plane.
    const float halfHeight = nearPlane * std::tan(fovY / 2.0f);
    const float halfWidth = halfHeight * aspect;

    const ezse::Vec4 nearCenter = PerspectiveDivide(projection * ezse::Vec4{ 0.0f, 0.0f, -nearPlane, 1.0f });
    Check(NearlyEqual(nearCenter, ezse::Vec4{ 0.0f, 0.0f, -1.0f, 1.0f }),
        "Perspective maps the center of the near plane to depth -1");

    const ezse::Vec4 farCenter = PerspectiveDivide(projection * ezse::Vec4{ 0.0f, 0.0f, -farPlane, 1.0f });
    Check(NearlyEqual(farCenter, ezse::Vec4{ 0.0f, 0.0f, 1.0f, 1.0f }),
        "Perspective maps the center of the far plane to depth 1");

    const ezse::Vec4 nearTop = PerspectiveDivide(projection * ezse::Vec4{ 0.0f, halfHeight, -nearPlane, 1.0f });
    Check(NearlyEqual(nearTop, ezse::Vec4{ 0.0f, 1.0f, -1.0f, 1.0f }),
        "Perspective maps the top edge of the near plane to y = 1");

    const ezse::Vec4 nearRight = PerspectiveDivide(projection * ezse::Vec4{ halfWidth, 0.0f, -nearPlane, 1.0f });
    Check(NearlyEqual(nearRight, ezse::Vec4{ 1.0f, 0.0f, -1.0f, 1.0f }),
        "Perspective maps the right edge of the near plane to x = 1");

    const ezse::Vec4 clip = projection * ezse::Vec4{ 3.0f, 4.0f, -5.0f, 1.0f };
    Check(std::abs(clip.w - 5.0f) <= 0.0001f,
        "Perspective puts the distance to the camera (-z) in w");
}

} // namespace

int main() {
    TestDefaultValue();
    TestVec3Add();
    TestVec3Negate();
    TestVec3Scale();
    TestMultiply();
    TestTranslation();
    TestScale();
    TestRotationX();
    TestRotationY();
    TestRotationZ();
    TestRotationOrder();
    TestTransformVector();
    TestTransformDirection();
    TestPerspective();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
