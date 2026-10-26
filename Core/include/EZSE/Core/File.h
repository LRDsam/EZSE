#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace ezse {

// Reads the whole file as text.
// Returns nothing (and logs why) if the file cannot be read.
std::optional<std::string> ReadTextFile(const std::filesystem::path& path);

} // namespace ezse
