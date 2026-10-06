include(Files.cmake)
include(Synchronization.cmake)
include(Fonts.cmake)

add_library(ren_platform STATIC
    SDL/Process.cpp
    SDL/Desktop.cpp
    SDL/Runtime.cpp
    SDL/TextInput.cpp)
target_include_directories(ren_platform PRIVATE "${PROJECT_SOURCE_DIR}/Code/wwlib" "${PROJECT_SOURCE_DIR}/Code/wwdebug")
target_include_directories(ren_platform PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(ren_platform PUBLIC cxx_std_23)
target_link_libraries(ren_platform PUBLIC ren_files ren_sync SDL3::SDL3-static)
if(WIN32)
    target_link_libraries(ren_platform PRIVATE iphlpapi)
    target_sources(ren_platform PRIVATE Windows/Media.cpp Windows/NativeWindow.cpp Windows/Debug.cpp Windows/Assertion.cpp Windows/Text.cpp Windows/SystemInfo.cpp Windows/Calendar.cpp Windows/Network.cpp Windows/UISettings.cpp Windows/Terminal.cpp Windows/Process.cpp)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(BLKID REQUIRED IMPORTED_TARGET blkid)
    target_link_libraries(ren_platform PRIVATE PkgConfig::BLKID)
    target_sources(ren_platform PRIVATE Linux/Media.cpp SDL/Debug.cpp SDL/Text.cpp Linux/SystemInfo.cpp POSIX/Calendar.cpp Linux/Network.cpp SDL/UISettings.cpp POSIX/Terminal.cpp Linux/Process.cpp)
endif()
add_subdirectory(SDL/GPU)

add_library(ren_audio STATIC SDL/Miles.cpp)
add_library(Vendor::Miles ALIAS ren_audio)
target_include_directories(ren_audio PUBLIC "${PROJECT_SOURCE_DIR}/Platform/Audio/Include")
target_include_directories(ren_audio PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(ren_audio PRIVATE cxx_std_23)
target_compile_definitions(ren_audio PRIVATE _CRT_SECURE_NO_WARNINGS AL_ALEXT_PROTOTYPES)
target_link_libraries(ren_audio PRIVATE SDL3::SDL3-static OpenAL::OpenAL ren_audio_decoders)
