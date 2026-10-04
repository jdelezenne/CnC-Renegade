cmake_minimum_required(VERSION 4.2)

add_library(libbinkdec STATIC
    Src/avfft.c
    Src/BinkAudio.cpp
    Src/BinkDecoder.cpp
    Src/BinkVideo.cpp
    Src/BitReader.cpp
    Src/dct.c
    Src/dct32.c
    Src/fft.c
    Src/FileStream.cpp
    Src/HuffmanVLC.cpp
    Src/LogError.cpp
    Src/mdct.c
    Src/rdft.c
    Src/Util.cpp)
add_library(Vendor::BinkDecoder ALIAS libbinkdec)
target_include_directories(libbinkdec PUBLIC Include)
target_compile_features(libbinkdec PRIVATE cxx_std_17)
target_compile_definitions(libbinkdec PRIVATE _CRT_SECURE_NO_WARNINGS)
