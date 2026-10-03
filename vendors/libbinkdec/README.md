# libbinkdec

Small Bink 1 video and audio decoder, built as a static library by CMake.
No FFmpeg libraries or proprietary Bink SDK are required.

Upstream: https://github.com/FriskTheFallenHuman/libbinkdec
Revision: 083d22bf352f734e35d20e756a8397396b1f2777
License: LGPL 2.1 or later; see COPYING and the source headers.

The include/ and src/ directories are copied from upstream. The CMake file
builds only the decoder needed by Renegade, without packaging or DLL targets.

Local changes (2026-10-03) in BinkDecoder.cpp and BinkAudio.cpp reject empty
video headers, bound PCM buffer writes, preserve each packet's actual PCM
length for audio synchronization, and limit the public YUV output to three
planes when a file also contains alpha.
