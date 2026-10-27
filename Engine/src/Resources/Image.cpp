#include "EZSE/Engine/Resources/Image.h"

#include "EZSE/Core/File.h"
#include "EZSE/Core/Log.h"

// We decode from memory only; our own code reads the files (no fopen, and paths with any
// characters work).
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ezse {

std::optional<Image> DecodeImage(std::span<const std::uint8_t> fileBytes) {
    stbi_set_flip_vertically_on_load(1);
    Image image{};
    int channelsInFile = 0;

    stbi_uc* imageData = stbi_load_from_memory(
        fileBytes.data(),
        static_cast<int>(fileBytes.size()),
        &image.width,
        &image.height,
        &channelsInFile,
        4);

    if (imageData == nullptr) {
        LogError("Failed to decode image: {}", stbi_failure_reason());
        return std::nullopt;
    }
    stbi_uc* imageDataEnd = imageData +
        std::size_t{4} *
        static_cast<std::size_t>(image.width) *
        static_cast<std::size_t>(image.height);
    image.pixels = std::vector<std::uint8_t>(imageData, imageDataEnd);
    stbi_image_free(imageData);
    return image;
}

std::optional<Image> ReadImageFile(const std::filesystem::path& path) {
    std::optional<std::vector<std::uint8_t>> binaryData = ReadBinaryFile(path);
    if (!binaryData.has_value()) {
        return std::nullopt;
    }
    std::optional<Image> image = DecodeImage(binaryData.value());
    if (!image.has_value()) {
        LogError("Failed to decode image '{}'", path.string());
        return std::nullopt;
    }
    return image;
}

} // namespace ezse
