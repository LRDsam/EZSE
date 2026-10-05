#include "EZSE/Core/Log.h"

#include <iostream>

namespace ezse {

namespace {

std::string_view LevelName(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace

void LogMessage(LogLevel level, std::string_view message) {
    std::cout << std::format("{:<7} {}\n", std::format("[{}]", LevelName(level)), message);
}

} // namespace ezse
