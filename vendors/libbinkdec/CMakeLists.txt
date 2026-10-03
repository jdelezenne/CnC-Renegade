cmake_minimum_required(VERSION 4.2)

add_library(libbinkdec STATIC
    src/avfft.c
    src/BinkAudio.cpp
    src/BinkDecoder.cpp
    src/BinkVideo.cpp
    src/BitReader.cpp
    src/dct.c
    src/dct32.c
    src/fft.c
    src/FileStream.cpp
    src/HuffmanVLC.cpp
    src/LogError.cpp
    src/mdct.c
    src/rdft.c
    src/Util.cpp)
add_library(Vendor::BinkDecoder ALIAS libbinkdec)
target_include_directories(libbinkdec PUBLIC include)
target_compile_features(libbinkdec PRIVATE cxx_std_17)
target_compile_definitions(libbinkdec PRIVATE _CRT_SECURE_NO_WARNINGS)
