#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ezse {

// Reads the whole file as text.
// Returns nothing (and logs why) if the file cannot be read.
std::optional<std::string> ReadTextFile(const std::filesystem::path& path);

std::optional<std::vector<std::uint8_t>> ReadBinaryFile(const std::filesystem::path& path);

} // namespace ezse
