include(FetchContent)
FetchContent_Declare(SAL
    URL https://raw.githubusercontent.com/dotnet/runtime/5c4e2a301ec1dd8ef75a3ba98e5be1ff2cea0130/src/coreclr/pal/inc/rt/sal.h
    URL_HASH SHA256=e26ee611231a859b5a800eb2e1606e7ca03b8e3981d7c98cf1ca20c89b658b68
    DOWNLOAD_NO_EXTRACT TRUE
    SOURCE_SUBDIR Renegade)
FetchContent_MakeAvailable(SAL)
FetchContent_Declare(directxmath
    URL https://github.com/microsoft/DirectXMath/archive/e2f2b9bddbbc4fd0f6f63586f86b2279213b991d.tar.gz
    URL_HASH SHA256=8b8dcc3ed8e591785f5fcec509c7256728ddc407eb54780c014f558b4485cebe
    OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(directxmath)
add_library(Microsoft::DirectXMath ALIAS DirectXMath)
target_include_directories(DirectXMath SYSTEM INTERFACE "$<BUILD_INTERFACE:${sal_SOURCE_DIR}>")
