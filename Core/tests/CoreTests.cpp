#include "EZSE/Core/VirtualFileSystem.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failureCount = 0;

void Check(bool condition, const char* description) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failureCount;
    }
}

// A new, empty directory in the system's temporary directory, removed again at the end of the
// test.
class TemporaryDirectory {
public:
    TemporaryDirectory() {
        static int counter = 0;
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("ezse-core-tests-" + std::to_string(now) + "-" + std::to_string(counter++));
        std::filesystem::create_directories(m_path);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    const std::filesystem::path& Path() const { return m_path; }

    // Writes a file at a path relative to this directory, making the directories on the way.
    void Write(const std::filesystem::path& relativePath, std::string_view content) const {
        const std::filesystem::path path = m_path / relativePath;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary);
        file << content;
    }

private:
    std::filesystem::path m_path;
};

void TestValidVirtualPaths() {
    Check(ezse::IsValidVirtualPath("engine"), "One segment is valid");
    Check(ezse::IsValidVirtualPath("engine/shaders/lit.vert"), "A nested path is valid");
    Check(ezse::IsValidVirtualPath("core/my_mod-2/a.b.c"), "Digits, _ - and . are valid");
    Check(ezse::IsValidVirtualPath(".gitkeep"), "A segment may start with a dot");
}

void TestInvalidVirtualPaths() {
    Check(!ezse::IsValidVirtualPath(""), "An empty path is not valid");
    Check(!ezse::IsValidVirtualPath("Engine/lit.vert"), "Upper case is not valid");
    Check(!ezse::IsValidVirtualPath("engine\\lit.vert"), "A backslash is not valid");
    Check(!ezse::IsValidVirtualPath("my file.txt"), "A space is not valid");
    Check(!ezse::IsValidVirtualPath("caf\xC3\xA9.png"), "A character outside ASCII is not valid");
    Check(!ezse::IsValidVirtualPath("c:/x"), "A drive letter is not valid");
    Check(!ezse::IsValidVirtualPath("/engine"), "An absolute path is not valid");
    Check(!ezse::IsValidVirtualPath("engine/"), "A slash at the end is not valid");
    Check(!ezse::IsValidVirtualPath("engine//lit.vert"), "An empty segment is not valid");
    Check(!ezse::IsValidVirtualPath("engine/./lit.vert"), "A . segment is not valid");
    Check(!ezse::IsValidVirtualPath("engine/../secret"), "A .. segment is not valid");
    Check(!ezse::IsValidVirtualPath(".."), "Only .. is not valid");
}

void TestMountAndRead() {
    const TemporaryDirectory directory;
    directory.Write("shaders/lit.vert", "vertex");
    directory.Write("textures/pixel.bin", std::string_view("\x00\xFF\x10", 3));

    ezse::VirtualFileSystem fileSystem;
    Check(fileSystem.MountDirectory(directory.Path(), "engine"), "Mounting a directory works");
    Check(fileSystem.Exists("engine/shaders/lit.vert"), "A file is under its mount point");
    Check(!fileSystem.Exists("shaders/lit.vert"), "A file is not found without its mount point");

    const std::optional<std::string> text = fileSystem.ReadText("engine/shaders/lit.vert");
    Check(text.has_value() && *text == "vertex", "ReadText gives the content");

    const std::optional<std::vector<std::uint8_t>> bytes =
        fileSystem.ReadBinary("engine/textures/pixel.bin");
    Check(bytes.has_value() && *bytes == std::vector<std::uint8_t>{0x00, 0xFF, 0x10},
          "ReadBinary gives the bytes");
}

void TestLaterMountWins() {
    const TemporaryDirectory base;
    base.Write("textures/zombie.png", "base");
    base.Write("textures/only-base.png", "base");
    const TemporaryDirectory patch;
    patch.Write("textures/zombie.png", "patch");

    ezse::VirtualFileSystem fileSystem;
    fileSystem.MountDirectory(base.Path(), "core");
    fileSystem.MountDirectory(patch.Path(), "core");

    Check(fileSystem.ReadText("core/textures/zombie.png") == "patch",
          "A later mount replaces a file with the same path");
    Check(fileSystem.ReadText("core/textures/only-base.png") == "base",
          "A later mount keeps the files it does not have");
}

void TestMountPointsAreSeparate() {
    const TemporaryDirectory first;
    first.Write("item.txt", "first");
    const TemporaryDirectory second;
    second.Write("item.txt", "second");

    ezse::VirtualFileSystem fileSystem;
    fileSystem.MountDirectory(first.Path(), "core");
    fileSystem.MountDirectory(second.Path(), "mymod");

    Check(fileSystem.ReadText("core/item.txt") == "first" &&
              fileSystem.ReadText("mymod/item.txt") == "second",
          "The same file name under two mount points does not collide");
}

void TestNestedMountPoint() {
    const TemporaryDirectory directory;
    directory.Write("a.txt", "a");

    ezse::VirtualFileSystem fileSystem;
    Check(fileSystem.MountDirectory(directory.Path(), "core/assets"),
          "A mount point may have more than one segment");
    Check(fileSystem.Exists("core/assets/a.txt"), "A file is under a nested mount point");
}

void TestInvalidFilesAreSkipped() {
    const TemporaryDirectory directory;
    directory.Write("good.txt", "good");
    directory.Write("Upper.txt", "upper");
    directory.Write("with space.txt", "space");
    directory.Write("Folder/inside.txt", "folder");

    ezse::VirtualFileSystem fileSystem;
    Check(fileSystem.MountDirectory(directory.Path(), "test"),
          "Mounting works with invalid files in the directory");
    Check(fileSystem.Exists("test/good.txt"), "A valid file is mounted");
    Check(!fileSystem.Exists("test/upper.txt"), "A file with upper case is skipped, not renamed");
    Check(!fileSystem.Exists("test/folder/inside.txt"),
          "A file in a directory with upper case is skipped");
}

void TestMountFailures() {
    const TemporaryDirectory directory;
    directory.Write("a.txt", "a");

    ezse::VirtualFileSystem fileSystem;
    Check(!fileSystem.MountDirectory(directory.Path(), ""), "An empty mount point fails");
    Check(!fileSystem.MountDirectory(directory.Path(), "Engine"), "An invalid mount point fails");
    Check(!fileSystem.MountDirectory(directory.Path() / "missing", "engine"),
          "A missing directory fails");
    Check(!fileSystem.MountDirectory(directory.Path() / "a.txt", "engine"),
          "A file instead of a directory fails");
    Check(!fileSystem.Exists("a.txt") && !fileSystem.Exists("engine/a.txt"),
          "A failed mount adds nothing");
}

void TestReadMissing() {
    const TemporaryDirectory directory;
    directory.Write("a.txt", "a");

    ezse::VirtualFileSystem fileSystem;
    fileSystem.MountDirectory(directory.Path(), "test");

    Check(!fileSystem.ReadText("test/missing.txt").has_value(),
          "ReadText of a missing file gives nothing");
    Check(!fileSystem.ReadBinary("test/missing.txt").has_value(),
          "ReadBinary of a missing file gives nothing");
    Check(!fileSystem.ReadText("test/../test/a.txt").has_value(),
          "ReadText of an invalid path gives nothing");
    Check(!fileSystem.Exists("Test/a.txt"), "Exists is case-sensitive, also on Windows");
}

} // namespace

int main() {
    TestValidVirtualPaths();
    TestInvalidVirtualPaths();
    TestMountAndRead();
    TestLaterMountWins();
    TestMountPointsAreSeparate();
    TestNestedMountPoint();
    TestInvalidFilesAreSkipped();
    TestMountFailures();
    TestReadMissing();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
