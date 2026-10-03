# SDL3

SDL 3.4.18 is fetched from the official release archive and verified by SHA-256.
Upstream: https://github.com/libsdl-org/SDL/releases/tag/release-3.4.18
Revision: 829a65d769d935c4852f8159e964312c0957260a
License: zlib; the downloaded source includes LICENSE.txt.

CMake builds the static library with the game's MSVC runtime. Tests, examples,
installation rules, and the shared library are disabled. SDL GPU remains
available for the subsequent renderer migration.
