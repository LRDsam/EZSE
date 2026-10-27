#include "EZSE/Platform/FileSystem.h"

#include "EZSE/Core/Log.h"

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#else
    #error "GetExecutableDirectory is only written for Windows."
#endif

namespace ezse {

std::filesystem::path GetExecutableDirectory() {
    wchar_t buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        LogError("Could not get the executable directory, error code: {}", GetLastError());
        return std::filesystem::path{};
    }
    return std::filesystem::path{buffer}.parent_path();
}

} // namespace ezse
