#include "EZSE/Core/Log.h"

int main() {
    ezse::LogTrace("Starting Sandbox");
    ezse::LogInfo("Hello from EZSE Core");
    ezse::LogWarn("This is a warning with a value: {}", 42);
    ezse::LogError("This is an error with two values: {} and {}", 3.14f, "text");
    return 0;
}
