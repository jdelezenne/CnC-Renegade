
# Command & Conquer Renegade

This repository includes source code for Command & Conquer Renegade. This release provides support to the [Steam Workshop](https://steamcommunity.com/workshop/browse/?appid=2229890) for the game.


## Dependencies

The game build requires:

- DirectX SDK with the original D3D8/D3DX8 interfaces (`Code/DirectX` by default)
- SDL3, fetched by CMake from a pinned release (`Vendors/SDL3`)
- Bundled Bink decoder (`Vendors/LibBinkDec`, LGPL-2.1-or-later)
- RAD Miles Sound System SDK (`Code/Miles6` by default)
- GameSpy SDK (`Code/GameSpy` by default), when GameSpy integration is enabled
- Bundled GNU regex (`Vendors/Regex-0.12`)


## Compiling (Win32 Only)

Build with Visual Studio 2026 (Desktop development with C++, including ATL)
and CMake 4.2 or newer. The presets select the v145 compiler and Win32 platform.
No developer command prompt or custom toolchain file is needed.

```bat
cmake --preset Debug
cmake --build --preset Debug
cmake --preset Release
cmake --build --preset Release
```

The only presets are `Debug` and `Release`. Binaries are written to
`Binaries/Debug` and `Binaries/Release`. Build trees live in `Build/Debug` and
`Build/Release`; libraries stay inside their build tree. The game uses C++23 and
the static MSVC runtime. MSVC 14.51 selects `/std:c++23preview`, its supported
C++23 language mode.

SDK locations can be set with `-DREN_<SDK>_ROOT=<path>` at configure time:
`DIRECTX`, `MILES`, `GAMESPY`, and `REGEX`. Defaults use the paths listed above.
GNU regex may use either `gnu_regex.c`/`gnu_regex.h` or `regex.c`/`regex.h`.

GameSpy services can be disabled with `-DREN_ENABLE_GAMESPY=OFF`.
DirectX, Miles, and GNU regex remain required. Audio uses the real Miles runtime
from the installed game.

Bink movies use the bundled libbinkdec decoder, the game's Direct3D 8 renderer,
and SDL3 PCM audio playback. No Bink SDK, `binkw32.dll`, or FFmpeg installation
is required. Keep the original `Data/Movies` files alongside the other game data.

SDL3 3.4.18 is fetched from its official release archive with a SHA-256 check
and built statically. The first configure requires network access. SDL owns
the main window, event pump, gameplay keyboard/mouse/joystick input, game timer,
and movie audio. Existing control bindings keep their original key IDs.

Retained native implementations live under `Platform/Windows/`: the Windows
entry point, registry, single-instance handling, browser/IME message bridge,
and Direct3D 8 device/renderer implementation. The game still targets Windows;
Direct3D 8 rendering and Miles game audio remain during this stage. SDL GPU
is the next renderer migration, using the SDL window and platform layer.

To use the compiled binaries, you must own the game. The C&C Ultimate Collection is available for purchase on [EA App](https://www.ea.com/en-gb/games/command-and-conquer/command-and-conquer-the-ultimate-collection/buy/pc) or [Steam](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/).

### Renegade

Use the CMake commands above, or open this repository as a CMake project in
Visual Studio 2026 and select Debug or Release. The game uses CMake directly;
its obsolete DSP/DSW build files have been removed.

The game still requires the original data files, Miles runtime and drivers,
and other runtime DLLs from an owned installation. BandTest is linked into the
executable. Build output includes the matching Scripts DLL; use it with the executable in a separate game directory containing
those assets.

### Free Dedicated Server
It’s possible to build the Windows version of the FDS (Free Dedicated Server) for Command & Conquer Renegade from the source code in this repository, just uncomment `#define FREEDEDICATEDSERVER` in [Code/Combat/specialbuilds.h](Code/Combat/specialbuilds.h) and perform a “Rebuild All” action on the Release config.


### Graphics configuration utility

`Code/Tools/WWConfig` retains the graphics/audio configuration utility sources.
It is not part of the current CMake build. The game can launch the installed
`WWConfig.exe` for graphics troubleshooting and driver warnings, so keep that
executable with the original game assets.

## Known Issues

The “Debug” configuration of the “Commando” project (the Renegade main project) will sometimes fail to link the final executable. This is due to Windows Defender incorrectly detecting RenegadeD.exe containing a virus (possibly due to the embedded browser code). Excluding the output `/Binaries/Debug/` folder found in the root of this repository in Windows Defender should resolve this for you.


## Contributing

This repository will not be accepting contributions (pull requests, issues, etc). If you wish to create changes to the source code and encourage collaboration, please create a fork of the repository under your GitHub user/organization space.


## Support

This repository is for preservation purposes only and is archived without support. 


## License

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.
