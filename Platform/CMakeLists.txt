add_library(ren_platform STATIC
    SDL/Runtime.cpp
    Windows/NativeWindow.cpp)
target_include_directories(ren_platform PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(ren_platform PUBLIC cxx_std_23)
target_link_libraries(ren_platform PUBLIC SDL3::SDL3-static)
