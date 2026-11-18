#include "EZSE/Core/VirtualFileSystem.h"

#include "EZSE/Core/File.h"
#include "EZSE/Core/Log.h"

#include <system_error>
#include <utility>

namespace ezse {

namespace {

bool IsAllowedCharacter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' ||
           c == '.' || c == '/';
}

bool IsValidSegment(std::string_view segment) {
    return !segment.empty() && segment != "." && segment != "..";
}

// The path as UTF-8 with / between the parts, also on Windows. Characters outside ASCII stay
// in, so IsValidVirtualPath rejects them and the warning can still show the path.
std::string ToGenericUtf8(const std::filesystem::path& path) {
    const std::u8string text = path.generic_u8string();
    return std::string(text.begin(), text.end());
}

} // namespace

bool IsValidVirtualPath(std::string_view path) {
    std::size_t segmentStart = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (!IsAllowedCharacter(path[i])) {
            return false;
        }
        if (path[i] == '/') {
            if (!IsValidSegment(path.substr(segmentStart, i - segmentStart))) {
                return false;
            }
            segmentStart = i + 1;
        }
    }
    // The last segment has no / after it. An empty path ends up here too.
    return IsValidSegment(path.substr(segmentStart));
}

bool VirtualFileSystem::MountDirectory(const std::filesystem::path& directory,
                                       std::string_view mountPoint) {
    if (!IsValidVirtualPath(mountPoint)) {
        LogError("Cannot mount '{}': invalid mount point '{}'", ToGenericUtf8(directory),
                 mountPoint);
        return false;
    }

    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        LogError("Cannot mount '{}' as '{}': not a directory", ToGenericUtf8(directory),
                 mountPoint);
        return false;
    }

    // Collected first, so that a directory that cannot be read halfway mounts nothing.
    std::vector<std::pair<std::string, std::filesystem::path>> found;
    std::filesystem::recursive_directory_iterator it(directory, error);
    const std::filesystem::recursive_directory_iterator end;
    for (; !error && it != end; it.increment(error)) {
        const std::filesystem::directory_entry& entry = *it;

        // A link could point outside the directory. Links to directories are not entered by
        // the iterator; this skips links to files. An entry that cannot be checked is skipped.
        std::error_code entryError;
        if (entry.is_symlink(entryError) || !entry.is_regular_file(entryError)) {
            continue;
        }

        const std::string relativePath = ToGenericUtf8(entry.path().lexically_relative(directory));
        const std::string virtualPath = std::string(mountPoint) + '/' + relativePath;
        if (!IsValidVirtualPath(virtualPath)) {
            LogWarn("Skipped '{}': not a valid virtual path (only a-z 0-9 _ - . and /)",
                    virtualPath);
            continue;
        }
        found.emplace_back(virtualPath, entry.path());
    }
    if (error) {
        LogError("Cannot mount '{}' as '{}': {}", ToGenericUtf8(directory), mountPoint,
                 error.message());
        return false;
    }

    for (auto& [virtualPath, realPath] : found) {
        m_files.insert_or_assign(std::move(virtualPath), std::move(realPath));
    }
    return true;
}

bool VirtualFileSystem::Exists(std::string_view path) const {
    return m_files.contains(std::string(path));
}

std::optional<std::string> VirtualFileSystem::ReadText(std::string_view path) const {
    const std::filesystem::path* realPath = Find(path);
    if (realPath == nullptr) {
        return std::nullopt;
    }
    return ReadTextFile(*realPath);
}

std::optional<std::vector<std::uint8_t>> VirtualFileSystem::ReadBinary(
    std::string_view path) const {
    const std::filesystem::path* realPath = Find(path);
    if (realPath == nullptr) {
        return std::nullopt;
    }
    return ReadBinaryFile(*realPath);
}

const std::filesystem::path* VirtualFileSystem::Find(std::string_view path) const {
    if (!IsValidVirtualPath(path)) {
        LogError("Invalid virtual path '{}'", path);
        return nullptr;
    }
    const auto it = m_files.find(std::string(path));
    if (it == m_files.end()) {
        LogError("No file '{}' is mounted", path);
        return nullptr;
    }
    return &it->second;
}

} // namespace ezse
