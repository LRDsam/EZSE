add_library(EZSEMath STATIC
    include/EZSE/Math/Mat4.h
    src/Mat4.cpp
)
add_library(EZSE::Math ALIAS EZSEMath)

target_include_directories(EZSEMath PUBLIC include)
