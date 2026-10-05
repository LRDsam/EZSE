add_library(EZSECore STATIC src/Log.cpp)
add_library(EZSE::Core ALIAS EZSECore)

target_include_directories(EZSECore PUBLIC include)
