#pragma once

#include <filesystem>

namespace ezse {

// The folder that holds the running executable; empty (and logged) if it cannot be found. Written
// for Windows only so far.
std::filesystem::path GetExecutableDirectory();

} // namespace ezse
