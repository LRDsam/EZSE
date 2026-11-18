#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ezse {

// A virtual path: only a-z 0-9 _ - . and /, relative, and no empty, "." or ".." segments.
bool IsValidVirtualPath(std::string_view path);

// One tree of virtual paths over real directories. Mount at startup, read afterwards.
class VirtualFileSystem {
public:
    // Adds every file under directory as mountPoint/<relative path>. A file that was
    // already mounted under that path is replaced: later mounts win. Files whose
    // path is not valid are skipped with a warning; symbolic links are not followed.
    // Returns false (and mounts nothing) if the mount point is not valid, or the
    // directory does not exist or cannot be read.
    bool MountDirectory(const std::filesystem::path& directory, std::string_view mountPoint);

    bool Exists(std::string_view path) const;

    // Returns nothing (and logs why) if the path is not valid or not mounted.
    std::optional<std::string> ReadText(std::string_view path) const;
    std::optional<std::vector<std::uint8_t>> ReadBinary(std::string_view path) const;

private:
    // The real file for a virtual path, or nullptr (and logs why) if the path is not valid or
    // not mounted.
    const std::filesystem::path* Find(std::string_view path) const;

    // Virtual path -> real file on disk.
    std::unordered_map<std::string, std::filesystem::path> m_files;
};

} // namespace ezse
