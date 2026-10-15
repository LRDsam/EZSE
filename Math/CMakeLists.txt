add_library(EZSEMath STATIC
    include/EZSE/Math/Mat4.h
    include/EZSE/Math/Transform.h
    include/EZSE/Math/Vec3.h
    include/EZSE/Math/Vec4.h
    src/Mat4.cpp
    src/Transform.cpp
    src/Vec3.cpp
)
add_library(EZSE::Math ALIAS EZSEMath)

target_include_directories(EZSEMath PUBLIC include)

add_executable(MathTests tests/MathTests.cpp)
target_link_libraries(MathTests PRIVATE EZSE::Math)
