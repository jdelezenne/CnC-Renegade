
# Command & Conquer Renegade

This repository includes source code for Command & Conquer Renegade. This release provides support to the [Steam Workshop](https://steamcommunity.com/workshop/browse/?appid=2229890) for the game.


## Dependencies

If you wish to rebuild the source code and tools successfully you will need to find or write new replacements (or remove the code using them entirely) for the following libraries;

- DirectX SDK (Version 8.0 or higher) (expected path `\Code\DirectX\`)
- Bink video decoder - bundled in `vendors/libbinkdec` (LGPL-2.1-or-later)
- RAD Miles Sound System SDK - (expected path `\Code\Miles6\`)
- NvDXTLib SDK - (expected path `\Code\NvDXTLib\`)
- Lightscape SDK - (expected path `\Code\Lightscape\`)
- Umbra SDK - (expected path `\Code\Umbra\`)
- GameSpy SDK - (expected path `\Code\GameSpy\`)
- GNU Regex - (expected path `\Code\WWLib\`)
- SafeDisk API - (expected path `\Code\Launcher\SafeDisk\`)
- Microsoft Cab Archive Library - (expected path `\Code\Installer\Cab\`)
- RTPatch Library - (expected path `\Code\Installer\`)
- Java Runtime Headers - (expected path `\Code\Tools\RenegadeGR\`)


## Compiling (Win32 Only)

Build with Visual Studio 2026 (Desktop development with C++, including ATL)
and CMake 4.2 or newer. The presets select the v145 compiler and Win32 platform.
No developer command prompt or custom toolchain file is needed.

```bat
cmake --preset debug
cmake --build --preset debug
cmake --preset release
cmake --build --preset release
```

The only presets are `debug` and `release`. Binaries are written to
`out/build/debug/bin/Debug` and `out/build/release/bin/Release`; libraries use
the corresponding `lib/<configuration>` directory. The game uses C++17 and
the static MSVC runtime.

SDK locations can be set with `-DREN_<SDK>_ROOT=<path>` at configure time:
`DIRECTX`, `MILES`, `GAMESPY`, `REGEX`, and `UMBRA`. Defaults use the
original paths listed above, except GNU regex uses `vendors/regex-0.12`.
GNU regex may use either `gnu_regex.c`/`gnu_regex.h` or `regex.c`/`regex.h`.

GameSpy services can be disabled with `-DREN_ENABLE_GAMESPY=OFF`.
Umbra is disabled by default and can be enabled with `-DREN_ENABLE_UMBRA=ON`.
DirectX, Miles, and GNU regex remain required. Audio uses the real Miles runtime
from the installed game.

Bink movies use the bundled libbinkdec decoder, the game's Direct3D 8 renderer,
and Windows PCM audio playback. No Bink SDK, `binkw32.dll`, or FFmpeg installation
is required. Keep the original `Data/Movies` files alongside the other game data.

To use the compiled binaries, you must own the game. The C&C Ultimate Collection is available for purchase on [EA App](https://www.ea.com/en-gb/games/command-and-conquer/command-and-conquer-the-ultimate-collection/buy/pc) or [Steam](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/).

### Renegade

Use the CMake commands above, or open this repository as a CMake project in
Visual Studio 2026 and select Debug or Release. The original DSP/DSW files are
retained as historical source references; the CMake build uses the current
compiler directly.

The game still requires the original data files, Miles runtime and drivers,
and other runtime DLLs from an owned installation. Build output includes the
matching Scripts and BandTest DLLs. Use these together with the executable in
a separate game directory containing those assets.

### Free Dedicated Server
It’s possible to build the Windows version of the FDS (Free Dedicated Server) for Command & Conquer Renegade from the source code in this repository, just uncomment `#define FREEDEDICATEDSERVER` in [Combat\specialbuilds.h](Combat\specialbuilds.h) and perform a “Rebuild All” action on the Release config.


### Level Edit (Public Release)
To build the public release build of Level Edit, modify the LevelEdit project settings and add `PUBLIC_EDITOR_VER` to the preprocessor defines.


## Known Issues

The “Debug” configuration of the “Commando” project (the Renegade main project) will sometimes fail to link the final executable. This is due to Windows Defender incorrectly detecting RenegadeD.exe containing a virus (possibly due to the embedded browser code). Excluding the output `/Run/` folder found in the root of this repository in Windows Defender should resolve this for you.


## Contributing

This repository will not be accepting contributions (pull requests, issues, etc). If you wish to create changes to the source code and encourage collaboration, please create a fork of the repository under your GitHub user/organization space.


## Support

This repository is for preservation purposes only and is archived without support. 


## License

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.
