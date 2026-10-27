#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace ezse {

// The pixels of an image, 4 bytes per pixel (red, green, blue, alpha).
// Row by row, and the first row is the bottom row of the picture, as OpenGL expects.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;
};

// Decodes a PNG, JPG, BMP or TGA file that is already in memory.
// Returns nothing (and logs why) if the bytes are not an image stb_image can read.
std::optional<Image> DecodeImage(std::span<const std::uint8_t> fileBytes);

// Reads and decodes an image file. Returns nothing (and logs why) if that fails.
std::optional<Image> ReadImageFile(const std::filesystem::path& path);

} // namespace ezse
