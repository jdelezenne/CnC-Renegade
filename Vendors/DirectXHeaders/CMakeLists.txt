include(FetchContent)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(DXHEADERS_INSTALL ON)
FetchContent_Declare(directx-headers
    URL https://github.com/microsoft/DirectX-Headers/archive/adbd6f3ba40795c46a8d0f33af00bcb57ff0f0a4.tar.gz
    URL_HASH SHA256=38aedf08f2a6f775cc619639d30b109bd684742fda506ab510fcae1517b70adf
    OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(directx-headers)
