add_library(ren_platform STATIC
    sdl/runtime.cpp
    windows/native_window.cpp)
target_include_directories(ren_platform PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(ren_platform PUBLIC cxx_std_17)
target_link_libraries(ren_platform PUBLIC SDL3::SDL3-static)
