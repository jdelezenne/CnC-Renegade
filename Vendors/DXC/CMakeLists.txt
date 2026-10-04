include(FetchContent)
FetchContent_Declare(DXC
    URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip
    URL_HASH SHA256=ad31b1fc8443175d204f77a611fdb3ef2ec42759bdc2f1167368de24a4a7e7f1
    SOURCE_SUBDIR Renegade)
FetchContent_MakeAvailable(DXC)
add_executable(Vendor::DXC IMPORTED GLOBAL)
set_target_properties(Vendor::DXC PROPERTIES IMPORTED_LOCATION "${dxc_SOURCE_DIR}/bin/x64/dxc.exe")
