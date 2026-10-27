#pragma once

#include <filesystem>

namespace ezse {

// The folder that holds the running executable.
std::filesystem::path GetExecutableDirectory();

} // namespace ezse
