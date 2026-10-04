include(FetchContent)
set(BUILD_TOOLS OFF)
set(BUILD_SAMPLE OFF)
set(BUILD_DX11 OFF)
set(BUILD_DX12 OFF)
set(BC_USE_OPENMP OFF)
FetchContent_Declare(DirectXTex
    URL https://github.com/microsoft/DirectXTex/archive/refs/tags/may2026.tar.gz
    URL_HASH SHA256=204ae68a66fb15d87867d836efbac9ae5aa799ca2a1867024d96a8168fa3651f)
FetchContent_MakeAvailable(DirectXTex)
