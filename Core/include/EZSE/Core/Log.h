#pragma once

#include <format>
#include <string_view>
#include <utility>

namespace ezse {

enum class LogLevel { Trace, Info, Warn, Error };

void LogMessage(LogLevel level, std::string_view message);

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
