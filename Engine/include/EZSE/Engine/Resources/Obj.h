#pragma once

#include "EZSE/Engine/Resources/MeshData.h"

#include <filesystem>
#include <optional>
#include <string_view>

namespace ezse {

std::optional<MeshData> ParseObj(std::string_view text);

} // namespace ezse