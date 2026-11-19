#include "EZSE/Core/File.h"

#include "EZSE/Core/Log.h"

#include <fstream>

namespace ezse {

namespace {

// Reads the whole file into a container of bytes (std::string or std::vector<std::uint8_t>) with
// one read of the right size, instead of character by character.
template <typename Container>
std::optional<Container> ReadWholeFile(const std::filesystem::path& path) {
    // Opened at the end (ate), so the position is the size.
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        LogError("Cannot open file '{}'", path.string());
        return std::nullopt;
    }
    const std::streamoff size = file.tellg();
    if (size < 0) {
        LogError("Cannot read the size of file '{}'", path.string());
        return std::nullopt;
    }
    file.seekg(0);

    Container content(static_cast<std::size_t>(size), 0);
    if (size > 0 && !file.read(reinterpret_cast<char*>(content.data()), size)) {
        LogError("Cannot read file '{}'", path.string());
        return std::nullopt;
    }
    return content;
}

} // namespace

std::optional<std::string> ReadTextFile(const std::filesystem::path& path) {
    return ReadWholeFile<std::string>(path);
}

std::optional<std::vector<std::uint8_t>> ReadBinaryFile(const std::filesystem::path& path) {
    return ReadWholeFile<std::vector<std::uint8_t>>(path);
}

} // namespace ezse
