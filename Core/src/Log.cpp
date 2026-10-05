#include "EZSE/Core/Log.h"

#include <iostream>

namespace ezse {

void Log(std::string_view message) {
    std::cout << message << '\n';
}

} // namespace ezse
