add_library(EZSECore STATIC
    include/EZSE/Core/Assert.h
    include/EZSE/Core/File.h
    include/EZSE/Core/Log.h
    include/EZSE/Core/VirtualFileSystem.h
    src/File.cpp
    src/Log.cpp
    src/VirtualFileSystem.cpp
)
add_library(EZSE::Core ALIAS EZSECore)

target_include_directories(EZSECore PUBLIC include)
target_compile_definitions(EZSECore PUBLIC
    $<$<CONFIG:Debug>:EZSE_DEBUG>
    $<$<CONFIG:Debug>:EZSE_ENABLE_ASSERTS>
)

if(EZSE_BUILD_TESTS)
    add_executable(CoreTests tests/CoreTests.cpp)
    target_link_libraries(CoreTests PRIVATE EZSE::Core)
endif()
