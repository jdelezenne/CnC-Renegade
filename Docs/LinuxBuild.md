# Linux Build

Native Linux builds use Clang and CMake presets. The complete Release executable and Scripts shared library build successfully with Clang 21 on Ubuntu 26.04. Runtime validation is in progress; remaining port work is tracked in [LinuxPort.md](LinuxPort.md).

## Tools and system packages

Install CMake 4.2 or newer, Clang with a C++23 standard library, Ninja and pkg-config. Both `clang` and `clang++` must be on PATH. The Windows clang-cl requirement for LLVM 23 does not apply to Linux. Use the same Linux commands from an Ubuntu terminal or an Ubuntu WSL shell.

For the SDL desktop, input and audio backends on Ubuntu:

```sh
sudo apt install ninja-build pkg-config libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev libxss-dev libxtst-dev libwayland-dev libxkbcommon-dev libasound2-dev libpulse-dev libudev-dev libblkid-dev libvulkan-dev
```

Runtime rendering requires a working Vulkan driver. WSL graphical testing also requires WSLg. The current WSL Vulkan device enumeration exposes only llvmpipe (CPU rendering), so WSL checks do not establish hardware performance parity. Native Ubuntu runtime validation with a hardware Vulkan driver remains part of the port plan.

## SDK headers and fetched dependencies

Supply an extracted DirectX SDK containing `d3d8.h`, `d3dx8.h`, `d3dx8math.inl` and `dsound.h`. These headers describe the existing D3D8 interfaces used by the SDL GPU shim. The build does not link the original D3D8 or D3DX8 runtime libraries.

Set `REN_DIRECTX_ROOT` to the extracted SDK directory, containing its `include`, `Include` or `INCLUDE` directory. This can be an environment variable or a CMake cache argument. The preset contains no host-specific SDK paths.

CMake fetches pinned SDL3, SDL3_ttf, SDL3_image, OpenAL Soft, audio decoders, DirectXTex, DirectXMath, DirectX-Headers and DXC dependencies. GNU regex and the Bink decoder are bundled. The first configure requires network access. GameSpy is disabled in the Linux preset because its original binary SDK is not a native Linux dependency.

## Configure and build

Run from the repository root:

```sh
cmake --preset linux-clang -DREN_DIRECTX_ROOT=/path/to/extracted/sdk
cmake --build --preset linux-clang-debug
cmake --build --preset linux-clang-release
```

The configure argument can be omitted when `REN_DIRECTX_ROOT` is already set in the environment.

The Debug and Release configurations share `Build/Linux-Clang-x64`. Executables and matching script libraries use `Binaries/Linux-Clang-x64/Debug` and `Binaries/Linux-Clang-x64/Release`. Run with the owned game installation as the working directory; game data stays read-only and user settings use the SDL preference folder.
