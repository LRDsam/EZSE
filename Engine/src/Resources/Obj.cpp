#include "EZSE/Engine/Resources/Obj.h"

#include "EZSE/Core/File.h"
#include "EZSE/Core/Log.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <unordered_map>
#include <vector>

namespace ezse {

namespace {

constexpr std::string_view c_whitespace = " \t";

struct ObjCorner {
    std::uint32_t position = 0;
    std::uint32_t uv = 0;
    std::uint32_t normal = 0;
    bool operator==(const ObjCorner&) const = default;
};

struct ObjCornerHash {
    std::size_t operator()(const ObjCorner& corner) const {
        return static_cast<std::size_t>(corner.position) * 73856093u
            ^ static_cast<std::size_t>(corner.uv) * 19349663u
            ^ static_cast<std::size_t>(corner.normal) * 83492791u;
    }
};

std::string_view NextLine(std::string_view& text) {
    const std::size_t position = text.find('\n');
    std::string_view line = text.substr(0, position);
    if (line.ends_with('\r')) {
        line.remove_suffix(1);
    }
    if (position == std::string_view::npos) {
        text = {};
    }
    else {
        text.remove_prefix(position + 1);
    }
    return line;
}

std::string_view NextToken(std::string_view& line) {
    const std::size_t begin = line.find_first_not_of(c_whitespace);
    if (begin == std::string_view::npos) {
        line = {};
        return {};
    }
    const std::size_t end = line.find_first_of(c_whitespace, begin);
    const std::string_view token = line.substr(begin, end - begin);
    if (end == std::string_view::npos) {
        line = {};
    }
    else {
        line.remove_prefix(end);
    }
    return token;
}

bool ParseFloat(std::string_view token, float& value) {
    const char* first = token.data();
    const char* last = token.data() + token.size();
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }
    return true;
}

template <std::size_t N>
bool ParseFloats(std::string_view& line, std::array<float, N>& values) {
    for (float& axis : values) {
        const std::string_view token = NextToken(line);
        if (!ParseFloat(token, axis)) {
            return false;
        }
    }
    return true;
}

bool ParseIndex(std::string_view token, std::uint32_t& value) {
    const char* first = token.data();
    const char* last = token.data() + token.size();
    const std::from_chars_result result = std::from_chars(first, last, value);
    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }
    return true;
}

bool ParseCorner(std::string_view token, ObjCorner& corner) {
    std::size_t location = token.find('/');
    if (location == std::string_view::npos) {
        return false;
    }
    if (!ParseIndex(token.substr(0, location), corner.position)) {
        return false;
    }
    token.remove_prefix(location + 1);
    location = token.find('/');
    if (location == std::string_view::npos) {
        return false;
    }
    if (!ParseIndex(token.substr(0, location), corner.uv)) {
        return false;
    }
    token.remove_prefix(location + 1);
    return ParseIndex(token, corner.normal);
}

} // namespace

std::optional<MeshData> ParseObj(std::string_view text) {
    int lineNumber = 0;
    std::vector<std::array<float, 3>> positions;
    std::vector<std::array<float, 3>> normals;
    std::vector<std::array<float, 2>> uvs;
    std::unordered_map<ObjCorner, std::uint32_t, ObjCornerHash> vertexIndices;
    MeshData mesh;
    while (!text.empty()) {
        lineNumber++;
        std::string_view line = NextLine(text);
        const std::string_view fullLine = line;
        const std::string_view keyword = NextToken(line);
        if (keyword == "v") {
            std::array<float, 3> position{};
            if (!ParseFloats(line, position)) {
                LogError("Failed to parse OBJ line {}: '{}'", lineNumber, fullLine);
                return std::nullopt;
            }
            positions.push_back(position);
        }
        else if (keyword == "vn") {
            std::array<float, 3> normal{};
            if (!ParseFloats(line, normal)) {
                LogError("Failed to parse OBJ line {}: '{}'", lineNumber, fullLine);
                return std::nullopt;
            }
            normals.push_back(normal);
        }
        else if (keyword == "vt") {
            std::array<float, 2> uv{};
            if (!ParseFloats(line, uv)) {
                LogError("Failed to parse OBJ line {}: '{}'", lineNumber, fullLine);
                return std::nullopt;
            }
            uvs.push_back(uv);
        }
        else if (keyword == "f") {
            // The face becomes a fan of triangles: from the third corner on, every corner adds
            // the triangle (first, previous, current).
            std::uint32_t first = 0;
            std::uint32_t previous = 0;
            int cornerCount = 0;
            for (std::string_view token = NextToken(line); !token.empty(); token = NextToken(line)) {
                ObjCorner corner;
                if (!ParseCorner(token, corner)) {
                    LogError("Failed to parse OBJ line {}: invalid corner '{}'", lineNumber, token);
                    return std::nullopt;
                }
                // OBJ indices start at 1 and may only refer to what has already been read.
                if (corner.position == 0 || corner.position > positions.size() ||
                    corner.uv == 0 || corner.uv > uvs.size() ||
                    corner.normal == 0 || corner.normal > normals.size()) {
                    LogError("Failed to parse OBJ line {}: index out of range in '{}'",
                        lineNumber, token);
                    return std::nullopt;
                }

                // A corner that was seen before reuses its Vertex; a new one adds a Vertex.
                std::uint32_t current = 0;
                const auto found = vertexIndices.find(corner);
                if (found != vertexIndices.end()) {
                    current = found->second;
                }
                else {
                    Vertex vertex{};
                    const std::array<float, 3>& position = positions[corner.position - 1];
                    const std::array<float, 3>& normal = normals[corner.normal - 1];
                    const std::array<float, 2>& uv = uvs[corner.uv - 1];
                    std::copy(position.begin(), position.end(), vertex.position);
                    std::copy(normal.begin(), normal.end(), vertex.normal);
                    std::copy(uv.begin(), uv.end(), vertex.uv);

                    current = static_cast<std::uint32_t>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                    vertexIndices.emplace(corner, current);
                }

                ++cornerCount;
                if (cornerCount == 1) {
                    first = current;
                }
                if (cornerCount >= 3) {
                    mesh.indices.push_back(first);
                    mesh.indices.push_back(previous);
                    mesh.indices.push_back(current);
                }
                previous = current;
            }
            if (cornerCount < 3) {
                LogError("Failed to parse OBJ line {}: a face needs at least 3 corners in '{}'",
                    lineNumber, fullLine);
                return std::nullopt;
            }
        }
    }
    if (mesh.indices.empty()) {
        LogError("Failed to parse OBJ: the text has no faces");
        return std::nullopt;
    }
    return mesh;
}

std::optional<MeshData> ReadObjFile(const std::filesystem::path& path) {
    std::optional<std::string> fileData = ReadTextFile(path);
    if (!fileData.has_value()) {
        return std::nullopt;
    }
    std::optional<MeshData> meshData = ParseObj(fileData.value());
    if (!meshData.has_value()) {
        LogError("Failed to load OBJ file '{}'", path.string());
        return std::nullopt;
    }
    return meshData;
}

} // namespace ezse
