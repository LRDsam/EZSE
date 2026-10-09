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

bool NearlyEqual(const ezse::Vec4& a, const ezse::Vec4& b) {
    return std::abs(a.x - b.x) <= 0.0001f && std::abs(a.y - b.y) <= 0.0001f &&
           std::abs(a.z - b.z) <= 0.0001f && std::abs(a.w - b.w) <= 0.0001f;
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
const float quarterTurn = std::numbers::pi_v<float> / 2.0f;

void TestRotationX() {
    // The Y axis ends up on Z, the Z axis on minus Y.
    ezse::Mat4 expected;
    expected.m[1][1] = 0.0f;
    expected.m[1][2] = 1.0f;
    expected.m[2][1] = -1.0f;
    expected.m[2][2] = 0.0f;

    Check(NearlyEqual(ezse::RotationX(0.0f), ezse::Mat4{}),
        "RotationX by zero is the identity");
    Check(NearlyEqual(ezse::RotationX(quarterTurn), expected),
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
    Check(NearlyEqual(ezse::RotationY(quarterTurn), expected),
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
    Check(NearlyEqual(ezse::RotationZ(quarterTurn), expected),
        "RotationZ by a quarter turn moves X onto Y");
    Check(NearlyEqual(ezse::RotationZ(0.3f) * ezse::RotationZ(0.5f), ezse::RotationZ(0.8f)),
        "two RotationZ multiply into the sum of their angles");
    Check(NearlyEqual(ezse::RotationZ(0.7f) * ezse::RotationZ(-0.7f), ezse::Mat4{}),
        "RotationZ by an angle and its opposite is the identity");
}

void TestRotationOrder() {
    Check(!NearlyEqual(ezse::RotationX(quarterTurn) * ezse::RotationY(quarterTurn),
                       ezse::RotationY(quarterTurn) * ezse::RotationX(quarterTurn)),
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
    Check(NearlyEqual(ezse::RotationZ(quarterTurn) * ezse::Vec4{ 1.0f, 0.0f, 0.0f, 1.0f },
                      ezse::Vec4{ 0.0f, 1.0f, 0.0f, 1.0f }),
        "RotationZ by a quarter turn moves the point (1, 0, 0) to (0, 1, 0)");
}

} // namespace

int main() {
    TestDefaultValue();
    TestMultiply();
    TestTranslation();
    TestScale();
    TestRotationX();
    TestRotationY();
    TestRotationZ();
    TestRotationOrder();
    TestTransformVector();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
