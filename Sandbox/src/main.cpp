#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

int main() {
    ezse::LogInfo("Hello from EZSE Core");

    int lightCount = 4;
    EZSE_ASSERT(lightCount <= 8, "Too many lights");

    ezse::LogInfo("Light count is valid: {}", lightCount);
    return 0;
}
