#pragma once

#include <format>
#include <string_view>
#include <utility>

namespace ezse {

// How serious a message is. Trace: details for debugging; Info: what happens (a file loaded);
// Warn: something is off, but the program goes on (a fallback is used); Error: something failed.
enum class LogLevel { Trace, Info, Warn, Error };

// Writes one line, "[LEVEL] message", to the console (std::cout). Not safe to call from two
// threads at once yet.
void LogMessage(LogLevel level, std::string_view message);

// Log, LogTrace, LogInfo, LogWarn and LogError format with std::format, checked when compiling:
// LogInfo("Loaded mesh '{}' (id {})", path, id).

template <typename... Args>
void Log(LogLevel level, std::format_string<Args...> format, Args&&... args) {
    LogMessage(level, std::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void LogTrace(std::format_string<Args...> format, Args&&... args) {
    Log(LogLevel::Trace, format, std::forward<Args>(args)...);
}

template <typename... Args>
void LogInfo(std::format_string<Args...> format, Args&&... args) {
    Log(LogLevel::Info, format, std::forward<Args>(args)...);
}

template <typename... Args>
void LogWarn(std::format_string<Args...> format, Args&&... args) {
    Log(LogLevel::Warn, format, std::forward<Args>(args)...);
}

template <typename... Args>
void LogError(std::format_string<Args...> format, Args&&... args) {
    Log(LogLevel::Error, format, std::forward<Args>(args)...);
}

} // namespace ezse
