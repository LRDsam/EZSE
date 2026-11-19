#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ezse {

// Reading a file by its real path on disk. Assets of the engine and the game go through the
// VirtualFileSystem instead; these are for what lies outside it (a path from the command line,
// later the folder for saves). The VirtualFileSystem uses them itself.

// Reads the whole file as text, unchanged (line endings stay as they are).
// Returns nothing (and logs why) if the file cannot be read.
std::optional<std::string> ReadTextFile(const std::filesystem::path& path);

// Reads the whole file as bytes. Returns nothing (and logs why) if the file cannot be read.
std::optional<std::vector<std::uint8_t>> ReadBinaryFile(const std::filesystem::path& path);

} // namespace ezse
