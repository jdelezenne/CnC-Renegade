include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
set(SDLTTF_VENDORED ON)
set(SDLTTF_SAMPLES OFF)
set(SDLTTF_INSTALL OFF)
set(SDLTTF_PLUTOSVG OFF)
FetchContent_Declare(SDLTTF
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b
    GIT_SUBMODULES external/freetype external/harfbuzz
    GIT_SUBMODULES_RECURSE FALSE)
FetchContent_MakeAvailable(SDLTTF)
