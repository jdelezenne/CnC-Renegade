
# Command & Conquer Renegade

This repository includes source code for Command & Conquer Renegade. This release provides support to the [Steam Workshop](https://steamcommunity.com/workshop/browse/?appid=2229890) for the game.


## Dependencies

The game build requires:

- DirectX SDK headers for the original D3D8/D3DX8 interfaces (`Code/DirectX` by default)
- SDL3, fetched by CMake from a pinned release (`Vendors/SDL3`)
- Bundled Bink decoder (`Vendors/LibBinkDec`, LGPL-2.1-or-later)
- OpenAL Soft 1.25.2 software mixer, fetched and built statically (`Vendors/OpenALSoft`)
- dr_wav/dr_mp3 decoders, fetched from a pinned revision (`Vendors/DrLibs`)
- GameSpy SDK (`Code/GameSpy` by default), when GameSpy integration is enabled
- Bundled GNU regex (`Vendors/Regex-0.12`)
- DirectXTex CPU texture helpers and DXC shader compiler, fetched from pinned releases


## Compiling on Windows

Use Visual Studio 2026 with Desktop development with C++ and ATL, CMake 4.2
or newer, and LLVM 23 or newer for clang-cl builds. The configure presets are
`windows-msvc-x64`, `windows-msvc-x86`, and `windows-clangcl`. Each provides
Debug and Release build presets. MSVC uses the Visual Studio generator;
clang-cl uses Ninja Multi-Config and the MSVC runtime and Windows SDK.

```bat
cmake --preset windows-msvc-x64
cmake --build --preset windows-msvc-x64-debug
cmake --build --preset windows-msvc-x64-release
cmake --preset windows-msvc-x86
cmake --build --preset windows-msvc-x86-release
```

For clang-cl from a terminal, use an x64 Visual Studio developer shell with
LLVM 23 and Ninja on PATH:

```bat
cmake --preset windows-clangcl
cmake --build --preset windows-clangcl-debug
cmake --build --preset windows-clangcl-release
```

Build trees are isolated under `Build/Windows-MSVC-x64`,
`Build/Windows-MSVC-x86`, and `Build/Windows-Clang-x64`. Executables and the
matching Scripts DLL go to `Binaries/<toolchain>/<config>`, for example
`Binaries/Windows-MSVC-x64/Release`. Libraries remain in their build tree.
The game uses C++23 and the static MSVC runtime. MSVC 14.51 selects
`/std:c++23preview`; clang-cl uses its C++23 mode.

SDK locations can be set with `-DREN_<SDK>_ROOT=<path>` at configure time:
`DIRECTX`, `GAMESPY`, and `REGEX`. Defaults use the paths listed above.
GNU regex may use either `gnu_regex.c`/`gnu_regex.h` or `regex.c`/`regex.h`.

GameSpy services can be disabled with `-DREN_ENABLE_GAMESPY=OFF`.
DirectX and GNU regex remain required. Game audio uses a Miles API compatibility
shim in `Platform/SDL/Miles.cpp`, with SDL3 output and OpenAL Soft mixing, spatial
audio, and reverb. WAV, IMA ADPCM, and MP3 decoding use dr_wav and dr_mp3.
No Miles SDK, runtime DLL, or audio drivers are required.

Bink movies use the bundled libbinkdec decoder, the game's Direct3D 8 renderer,
and SDL3 PCM audio playback. No Bink SDK, `binkw32.dll`, or FFmpeg installation
is required. Keep the original `Data/Movies` files alongside the other game data.

SDL3 3.4.18 is fetched from its official release archive with a SHA-256 check
and built statically. The first configure requires network access. SDL owns
the main window, event pump, gameplay keyboard/mouse/joystick input, game timer,
and all audio output. Existing control bindings keep their original key IDs.

Retained native implementations live under `Platform/Windows/`: the Windows
entry point, single-instance handling, browser/IME message bridge, and the
engine's Direct3D 8 wrapper. The Direct3D 8 compatibility renderer lives under
`Platform/SDL/GPU` and uses SDL GPU with D3D12 or Vulkan. D3DX texture and math
helpers are source-built; the original D3DX8 binary library is not required.

To use the compiled binaries, you must own the game. The C&C Ultimate Collection is available for purchase on [EA App](https://www.ea.com/en-gb/games/command-and-conquer/command-and-conquer-the-ultimate-collection/buy/pc) or [Steam](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/).

### Renegade

Use the CMake commands above, or open this repository as a CMake project in
Visual Studio 2026 and select the MSVC architecture and Debug or Release. The game uses CMake directly;
its obsolete DSP/DSW build files have been removed.

The game still requires the original data files and other runtime DLLs from
an owned installation. BandTest is linked into the
executable. Build output includes the matching Scripts DLL. Run the executable
with your installed game folder as its working directory.

For VS Code, set `renegade.gameDirectory` in `.vscode/settings.json` to your
installation folder (currently `D:\EA Games\Renegade`). With the Microsoft
C/C++ and CMake Tools extensions installed, select a configure preset and its
Debug or Release build preset, then select `renegade` as the launch target.
F5 builds and launches the selected target using the installed game folder as
its working directory. No game files are copied.

Settings are stored in `Settings.ini` under SDL's per-user preferences folder,
`%APPDATA%\Electronic Arts\Renegade` on Windows. The game starts with defaults;
there is no import from the registry or an older installation. Saves, input
profiles, logs, caches, and downloads are written beneath this folder. The
installation folder is used only to read assets and runtime DLLs.

The original WOLAPI and WOLBrowser binaries are not loaded. Their client interfaces
are retained for a compatible replacement supplied as `OnlineServices.dll` and
`OnlineBrowser.dll` beside the executable, loaded without COM registration.
Until a replacement is available, Westwood Online is unavailable; single-player
and LAN remain supported.

### Free Dedicated Server
It’s possible to build the Windows version of the FDS (Free Dedicated Server) for Command & Conquer Renegade from the source code in this repository, just uncomment `#define FREEDEDICATEDSERVER` in [Code/Combat/specialbuilds.h](Code/Combat/specialbuilds.h) and perform a “Rebuild All” action on the Release config.


### Graphics configuration utility

`Code/Tools/WWConfig` retains the graphics/audio configuration utility sources.
It is not part of the current CMake build.

## Contributing

This repository will not be accepting contributions (pull requests, issues, etc). If you wish to create changes to the source code and encourage collaboration, please create a fork of the repository under your GitHub user/organization space.


## Support

This repository is for preservation purposes only and is archived without support. 


## License

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.
