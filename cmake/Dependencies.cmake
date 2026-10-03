include_guard(GLOBAL)

function(ren_sdk_root name default_path)
    if(DEFINED ENV{REN_${name}_ROOT})
        set(default_path "$ENV{REN_${name}_ROOT}")
    endif()
    set(REN_${name}_ROOT "${default_path}" CACHE PATH "Root of the legacy ${name} SDK")
endfunction()

function(ren_find_dependencies)
    ren_sdk_root(DIRECTX "${PROJECT_SOURCE_DIR}/Code/DirectX")
    ren_sdk_root(MILES "${PROJECT_SOURCE_DIR}/Code/Miles6")
    ren_sdk_root(BINK "${PROJECT_SOURCE_DIR}/Code/BinkMovie")
    ren_sdk_root(REGEX "${PROJECT_SOURCE_DIR}/vendors/regex-0.12")
    ren_sdk_root(GAMESPY "${PROJECT_SOURCE_DIR}/Code/GameSpy")
    ren_sdk_root(UMBRA "${PROJECT_SOURCE_DIR}/Code/Umbra")

    find_path(REN_DIRECTX_INCLUDE_DIR d3dx8.h PATHS "${REN_DIRECTX_ROOT}/include" NO_DEFAULT_PATH)
    foreach(lib d3dx8 dinput dxguid dsound)
        find_library(REN_DIRECTX_${lib}_LIBRARY NAMES ${lib}
            PATHS "${REN_DIRECTX_ROOT}/lib" "${REN_DIRECTX_ROOT}/lib/x86" NO_DEFAULT_PATH)
    endforeach()
    find_path(REN_MILES_INCLUDE_DIR mss.h PATHS "${REN_MILES_ROOT}/include" NO_DEFAULT_PATH)
    find_library(REN_MILES_LIBRARY NAMES mss32
        PATHS "${REN_MILES_ROOT}/lib/win" "${REN_MILES_ROOT}/lib" "${REN_MILES_ROOT}/win" NO_DEFAULT_PATH)
    find_path(REN_BINK_INCLUDE_DIR bink.h PATHS "${REN_BINK_ROOT}" "${REN_BINK_ROOT}/include" NO_DEFAULT_PATH)
    find_library(REN_BINK_LIBRARY NAMES binkw32
        PATHS "${REN_BINK_ROOT}" "${REN_BINK_ROOT}/lib" "${REN_BINK_ROOT}/lib/win32" NO_DEFAULT_PATH)
    find_file(REN_REGEX_SOURCE NAMES gnu_regex.c regex.c PATHS "${REN_REGEX_ROOT}" NO_DEFAULT_PATH)
    find_path(REN_REGEX_INCLUDE_DIR NAMES gnu_regex.h regex.h PATHS "${REN_REGEX_ROOT}" NO_DEFAULT_PATH)
    find_path(REN_GAMESPY_HEADER_DIR gqueryreporting.h PATHS "${REN_GAMESPY_ROOT}" NO_DEFAULT_PATH)
    find_library(REN_GAMESPY_LIBRARY NAMES gamespy
        PATHS "${REN_GAMESPY_ROOT}/lib" "${REN_GAMESPY_ROOT}/lib/${CMAKE_BUILD_TYPE}"
        "${PROJECT_SOURCE_DIR}/Code/Libs/${CMAKE_BUILD_TYPE}" NO_DEFAULT_PATH)
    find_path(REN_UMBRA_INCLUDE_DIR umbra.hpp PATHS "${REN_UMBRA_ROOT}/interface" NO_DEFAULT_PATH)
    find_library(REN_UMBRA_LIBRARY NAMES umbra
        PATHS "${REN_UMBRA_ROOT}/lib/win32-x86" NO_DEFAULT_PATH)

    # Check headers as well as archives; runtime DLLs do not constitute an SDK.
    set(missing "")
    foreach(item DIRECTX_INCLUDE_DIR DIRECTX_d3dx8_LIBRARY DIRECTX_dinput_LIBRARY
            DIRECTX_dxguid_LIBRARY DIRECTX_dsound_LIBRARY MILES_INCLUDE_DIR MILES_LIBRARY
            GAMESPY_HEADER_DIR GAMESPY_LIBRARY REGEX_SOURCE REGEX_INCLUDE_DIR
            BINK_INCLUDE_DIR BINK_LIBRARY UMBRA_INCLUDE_DIR UMBRA_LIBRARY)
        if((item MATCHES "^(DIRECTX|MILES|REGEX)_") OR
           (REN_ENABLE_GAMESPY AND item MATCHES "^GAMESPY_") OR
           (REN_ENABLE_BINK AND item MATCHES "^BINK_") OR
           (REN_ENABLE_UMBRA AND item MATCHES "^UMBRA_"))
            if(NOT REN_${item})
                list(APPEND missing "REN_${item}")
            endif()
        endif()
    endforeach()
    if(REN_ENABLE_GAMESPY AND REN_GAMESPY_HEADER_DIR)
        foreach(header ghttp.h gcdkeyserver.h gcdkeyclient.h nonport.h gs_md5.h gs_patch_usage.h)
            if(NOT EXISTS "${REN_GAMESPY_HEADER_DIR}/${header}")
                list(APPEND missing "GameSpy/${header}")
            endif()
        endforeach()
        get_filename_component(sdk_name "${REN_GAMESPY_HEADER_DIR}" NAME)
        string(TOLOWER "${sdk_name}" sdk_name)
        if(NOT sdk_name STREQUAL "gamespy")
            list(APPEND missing "GameSpy headers must reside in a directory named GameSpy (source uses <GameSpy/...>)")
        endif()
    endif()
    if(REN_DIRECTX_INCLUDE_DIR)
        foreach(header d3d8.h dinput.h dsound.h)
            if(NOT EXISTS "${REN_DIRECTX_INCLUDE_DIR}/${header}")
                list(APPEND missing "DirectX/include/${header}")
            endif()
        endforeach()
    endif()
    if(REN_ENABLE_BINK AND REN_BINK_INCLUDE_DIR AND
       NOT EXISTS "${REN_BINK_INCLUDE_DIR}/rad.h")
        list(APPEND missing "Bink/rad.h")
    endif()
    if(missing)
        list(JOIN missing "\n  " details)
        message(FATAL_ERROR "Missing SDK dependencies:\n  ${details}\nSet REN_<SDK>_ROOT or disable an optional vendor through REN_ENABLE_<SDK>.")
    endif()
endfunction()

function(ren_import_sdk name library include_dir)
    add_library(${name} UNKNOWN IMPORTED GLOBAL)
    set_target_properties(${name} PROPERTIES
        IMPORTED_LOCATION "${library}" INTERFACE_INCLUDE_DIRECTORIES "${include_dir}")
endfunction()
