add_library(ren_fonts STATIC SDL/Fonts.cpp SDL/FontMetrics.cpp)
target_include_directories(ren_fonts PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(ren_fonts PUBLIC cxx_std_23)
target_link_libraries(ren_fonts PRIVATE SDL3::SDL3-static SDL3_ttf::SDL3_ttf-static Freetype::Freetype)
