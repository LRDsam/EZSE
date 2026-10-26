#include "EZSE/Core/File.h"

#include "EZSE/Core/Log.h"

#include <fstream>
#include <iterator>

namespace ezse {

std::optional<std::string> ReadTextFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        LogError("Cannot open file '{}'", path.string());
        return std::nullopt;
    }
    return std::string(std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

} // namespace ezse
