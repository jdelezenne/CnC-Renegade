add_library(ren_platform STATIC
    SDL/Paths.cpp
    Windows/Files.cpp
    SDL/Runtime.cpp
    Windows/NativeWindow.cpp)
target_include_directories(ren_platform PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(ren_platform PUBLIC cxx_std_23)
target_link_libraries(ren_platform PUBLIC SDL3::SDL3-static)

add_library(ren_audio STATIC SDL/Miles.cpp)
add_library(Vendor::Miles ALIAS ren_audio)
target_include_directories(ren_audio PUBLIC "${PROJECT_SOURCE_DIR}/Platform/Audio/Include")
target_compile_features(ren_audio PRIVATE cxx_std_23)
target_compile_definitions(ren_audio PRIVATE _CRT_SECURE_NO_WARNINGS AL_ALEXT_PROTOTYPES)
target_link_libraries(ren_audio PRIVATE SDL3::SDL3-static OpenAL::OpenAL ren_audio_decoders)
