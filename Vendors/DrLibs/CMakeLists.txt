include(FetchContent)
FetchContent_Declare(DrLibs
    URL https://github.com/mackron/dr_libs/archive/dfe8377631000664666519fdb83da193fd8037f4.tar.gz
    URL_HASH SHA256=4654acb029f4f2a43ac2edb60c4cb09f40615b4b5bee9709954f910cb979e5fd
    SOURCE_SUBDIR Renegade)
FetchContent_MakeAvailable(DrLibs)
add_library(ren_audio_decoders INTERFACE)
target_include_directories(ren_audio_decoders INTERFACE "${drlibs_SOURCE_DIR}")
