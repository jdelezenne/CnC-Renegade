function(ren_add_legacy_target name kind)
    set(sources ${REN_${name}_SOURCES})
    if(name STREQUAL "wwlib")
        list(REMOVE_ITEM sources "${PROJECT_SOURCE_DIR}/Code/wwlib/gnu_regex.c")
        list(APPEND sources "${REN_REGEX_SOURCE}")
    elseif(name STREQUAL "renegade" AND NOT REN_ENABLE_GAMESPY)
        list(REMOVE_ITEM sources "${PROJECT_SOURCE_DIR}/Code/Commando/GameSpy_QnR.cpp")
        list(APPEND sources "${PROJECT_SOURCE_DIR}/cmake/stubs/gamespy-disabled.cpp")
    endif()
    if(kind STREQUAL "WIN32")
        add_executable(${name} WIN32 ${sources})
    else()
        add_library(${name} ${kind} ${sources})
    endif()
    ren_legacy_settings(${name})
    target_compile_options(${name} PRIVATE "$<$<COMPILE_LANGUAGE:C,CXX>:/W3;/MP4>")
    target_compile_features(${name} PRIVATE cxx_std_17)
    target_compile_definitions(${name} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
        WINVER=0x0A00 _WIN32_WINNT=0x0A00)
    target_compile_definitions(${name} PRIVATE
        REN_ENABLE_GAMESPY=$<BOOL:${REN_ENABLE_GAMESPY}>)
    if(kind STREQUAL "SHARED" OR kind STREQUAL "WIN32")
        target_link_options(${name} PRIVATE /DEBUG /MACHINE:I386)
        set_target_properties(${name} PROPERTIES DEBUG_POSTFIX D)
    endif()
endfunction()

set(REN_CORE_LIBRARIES wwdebug wwlib wwmath wwutil wwbitpack wwsaveload scontrol)
foreach(name IN LISTS REN_CORE_LIBRARIES)
    ren_add_legacy_target(${name} STATIC)
endforeach()
target_include_directories(wwlib PRIVATE "${REN_REGEX_INCLUDE_DIR}")
if(NOT EXISTS "${REN_REGEX_INCLUDE_DIR}/gnu_regex.h")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
    file(WRITE "${CMAKE_BINARY_DIR}/generated/gnu_regex.h" "#include \"regex.h\"\n")
    target_include_directories(wwlib PRIVATE "${CMAKE_BINARY_DIR}/generated")
endif()
ren_add_legacy_target(scripts SHARED)
ren_add_legacy_target(bandtest SHARED)
set_target_properties(scripts PROPERTIES OUTPUT_NAME Scripts)
set_target_properties(bandtest PROPERTIES OUTPUT_NAME BandTest)
target_link_libraries(scripts PRIVATE kernel32 user32 gdi32 winspool comdlg32 advapi32
    shell32 ole32 oleaut32 uuid odbc32 odbccp32)
target_link_libraries(bandtest PRIVATE kernel32 user32 gdi32 winspool comdlg32 advapi32
    shell32 ole32 oleaut32 uuid odbc32 odbccp32 ws2_32 winmm)

# Only stage the DX8 headers. The old SDK also ships Windows headers such as
# basetsd.h that must not override the Windows SDK selected by Visual Studio.
set(directx_headers "${CMAKE_BINARY_DIR}/generated/directx8")
file(MAKE_DIRECTORY "${directx_headers}")
file(GLOB dx8_headers "${REN_DIRECTX_INCLUDE_DIR}/d3d8*.h"
    "${REN_DIRECTX_INCLUDE_DIR}/dxfile.h" "${REN_DIRECTX_INCLUDE_DIR}/d3dx8*.h" "${REN_DIRECTX_INCLUDE_DIR}/d3dx8*.inl")
foreach(header IN LISTS dx8_headers)
    get_filename_component(filename "${header}" NAME)
    configure_file("${header}" "${directx_headers}/${filename}" COPYONLY)
endforeach()
foreach(lib d3dx8 dinput dxguid dsound)
    ren_import_sdk(Vendor::${lib} "${REN_DIRECTX_${lib}_LIBRARY}" "${directx_headers}")
endforeach()
# D3DX8 requests the discontinued single-thread CRT. Use the selected modern
# runtime and Microsoft's compatibility definitions for its C stdio imports.
set_property(TARGET Vendor::d3dx8 APPEND PROPERTY INTERFACE_LINK_OPTIONS
    /NODEFAULTLIB:libci /NODEFAULTLIB:libc)
set_property(TARGET Vendor::d3dx8 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
    legacy_stdio_definitions)
ren_import_sdk(Vendor::Miles "${REN_MILES_LIBRARY}" "${REN_MILES_INCLUDE_DIR}")
if(REN_ENABLE_GAMESPY)
    get_filename_component(gamespy_parent "${REN_GAMESPY_HEADER_DIR}" DIRECTORY)
    ren_import_sdk(Vendor::GameSpy "${REN_GAMESPY_LIBRARY_RELEASE}" "${gamespy_parent}")
    set_target_properties(Vendor::GameSpy PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_DEBUG "${REN_GAMESPY_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_RELEASE "${REN_GAMESPY_LIBRARY_RELEASE}")
endif()
set(REN_GAME_LIBRARIES wwnet ww3d2 wwphys wwaudio wwtranslatedb wwui combat binkmovie)
foreach(name IN LISTS REN_GAME_LIBRARIES)
    ren_add_legacy_target(${name} STATIC)
    target_link_libraries(${name} PRIVATE Vendor::d3dx8 Vendor::Miles)
endforeach()
target_link_libraries(binkmovie PRIVATE Vendor::BinkDecoder winmm)
target_include_directories(binkmovie PRIVATE "${PROJECT_SOURCE_DIR}/Code/wwaudio")
ren_add_legacy_target(renegade WIN32)
set_target_properties(renegade PROPERTIES OUTPUT_NAME Renegade)
# Scripts is loaded at runtime; BandTest exports use its import library.
add_dependencies(renegade scripts)
target_link_libraries(renegade PRIVATE ${REN_GAME_LIBRARIES} ${REN_CORE_LIBRARIES}
    bandtest Vendor::d3dx8 Vendor::dinput Vendor::dxguid Vendor::dsound
    Vendor::Miles kernel32 user32 gdi32 winspool comdlg32 advapi32
    shell32 ole32 oleaut32 uuid winmm vfw32 wsock32 version)
if(REN_ENABLE_GAMESPY)
    target_link_libraries(renegade PRIVATE Vendor::GameSpy)
endif()
