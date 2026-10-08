#include "EZSE/Math/Mat4.h"

#include <cmath>
#include <iostream>

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

// Built by hand so the tests do not depend on the Math functions they are checking.
ezse::Mat4 MakeTranslation(float x, float y, float z) {
    ezse::Mat4 result;
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
}

ezse::Mat4 MakeScale(float factor) {
    ezse::Mat4 result;
    result.m[0][0] = factor;
    result.m[1][1] = factor;
    result.m[2][2] = factor;
    return result;
}

void TestDefaultValue() {
    const ezse::Mat4 identity;

    Check(identity.m[0][0] == 1.0f && identity.m[1][1] == 1.0f &&
          identity.m[2][2] == 1.0f && identity.m[3][3] == 1.0f,
          "a default Mat4 has ones on the diagonal");
    Check(identity.m[3][0] == 0.0f && identity.m[0][3] == 0.0f,
          "a default Mat4 has zeros off the diagonal");
    Check(NearlyEqual(identity, ezse::Mat4{}), "two default Mat4s are nearly equal");
}

void TestMultiply() {
    const ezse::Mat4 identity;
    const ezse::Mat4 distinct{{
        { 1.0f,  2.0f,  3.0f,  4.0f},
        { 5.0f,  6.0f,  7.0f,  8.0f},
        { 9.0f, 10.0f, 11.0f, 12.0f},
        {13.0f, 14.0f, 15.0f, 16.0f},
    }};

    Check(NearlyEqual(identity * identity, identity), "identity * identity is the identity");
    Check(NearlyEqual(distinct * identity, distinct), "A * identity is A");
    Check(NearlyEqual(identity * distinct, distinct), "identity * A is A");

    Check(NearlyEqual(MakeTranslation(4.0f, 5.0f, 6.0f) * MakeTranslation(1.0f, 2.0f, 3.0f),
                      MakeTranslation(5.0f, 7.0f, 9.0f)),
          "two translations add up");

    // A * B applies B first. The two orders below only differ in the translation.
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

} // namespace

int main() {
    TestDefaultValue();
    TestMultiply();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
