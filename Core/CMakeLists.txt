add_library(EZSECore STATIC
    include/EZSE/Core/Assert.h
    include/EZSE/Core/File.h
    include/EZSE/Core/Log.h
    src/File.cpp
    src/Log.cpp
)
add_library(EZSE::Core ALIAS EZSECore)

target_include_directories(EZSECore PUBLIC include)
target_compile_definitions(EZSECore PUBLIC
    $<$<CONFIG:Debug>:EZSE_DEBUG>
    $<$<CONFIG:Debug>:EZSE_ENABLE_ASSERTS>
)
