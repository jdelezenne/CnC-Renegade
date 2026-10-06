# Linux Port Plan

This plan covers Linux, with macOS deferred until we have a Mac host. Each phase gets a separate reviewable change and relevant Windows regression checks. If an active feature requires a behavior decision, bring the concrete blocker to the user before changing it.

## Progress

Phases 1 and 2 are complete. Phase 1 was closed at the user's direction after the MSVC Debug build with `REN_ENABLE_WWDEBUG=OFF` passed; further Windows validation builds were stopped as requested. Phase 3 is complete: the native Release executable and Scripts module build and initialize. WSLg testing through Wayland now reaches the menu, loading, the in-game cinematic and gameplay with graphics and audio. Phase 4 retains local input validation, and phase 5 remains open for gameplay, saves, exit and native hardware validation. Phase 6 implements shared socket, LAN, NAT and server-control code; actual Windows/Linux game interoperability and bandwidth-provider integration remain pending. Linux Debug and clean packaging validation are also pending.

Update the checkboxes only after the work and its gate are complete. Record validation results and unresolved blockers under the relevant phase.

## 1 Finish the Windows cleanup checkpoint

- [x] Complete this phase.

Complete Debug with `WWDEBUG` on/off and Release validation. Preserve the original logging/assertion behavior. Keep the checkpoint separate from Linux changes.

## 2 Make Linux builds reproducible

- [x] Complete this phase.

Add Linux Clang presets for Debug and Release, using the existing folder/configuration conventions. Document the required system packages and remove dependencies on machine-specific paths. The build must attempt the complete game and script library.

**Validation:** the `linux-clang` preset configured successfully in Ubuntu WSL with Clang 23 and Ninja Multi-Config. Generated Debug and Release targets include the complete `renegade` executable and `scripts` shared library, with outputs under `Binaries/Linux-Clang-x64/<config>`. Windows preset discovery remains available on Windows. No game compilation was run for this phase. Setup is documented in [LinuxBuild.md](LinuxBuild.md).

## 3 Finish application and script loading

- [x] Complete this phase.

Share startup, the SDL event loop and shutdown; keep necessary OS code in platform sources. Add native shared-library loading with matching script exports. Preserve existing scripts and save formats.

- [x] Share SDL startup, events, focus handling, command-line arguments and shutdown across platforms.
- [x] Put native exception handling and process-instance locks behind the platform interface.
- [x] Use the existing SDL Scripts loader and native library names; remove the Windows-only Scripts entry point.
- [x] Link and initialize the complete native executable.

**Validation:** the shared startup, instance keeper, command-line parser and Scripts loader compile with native Linux Clang 23. The corresponding sources also passed scoped Windows Clang checks. Linux ASan/UBSan checks covered duplicate-instance rejection, `MULTI`, normal release and process-termination cleanup. A fresh harness loaded the actual native Scripts module, checked all required exports and the commands ABI, created/destroyed all 1,636 registered scripts (including `Test_Cinematic`), and unloaded it successfully. A focused parser check covered existing switches, IP/port, names/passwords containing spaces, password-option priority and preserving `argv`. Scratch fixtures are outside the repository. The complete native Release executable and Scripts module subsequently linked with Clang 21 on Ubuntu 26.04. WSLg startup reached Vulkan initialization, loaded fonts and opened the intro movie using the existing retail installation.

**Gate:** the native executable and script library link successfully and reach initialization. Met by the complete Release build and debugger-observed WSLg startup. Menu and gameplay checks belong to the remaining phases.

## 4 Resolve the remaining game and UI blockers

- [ ] Complete this phase.

Replace Windows calls in menus, file/server dialogs and useful console/debug tools through the platform interfaces. Finish the portable WOL placeholders and remove their COM/ATL dependency. Check CD verification and browser code for actual use before changing it.

- [x] Replace the disabled embedded WOL browser and its process polling with shared SDL URL launch; close the confirmation dialog after launch as requested.
- [x] Use the shared directory API for main-menu map searches and portable filename comparisons for input configurations.
- [x] Share the WOL interfaces, reference pointers and event-advisement implementation on Windows and Linux; retain the provider placeholder and game-facing WOL code.
- [x] Move optical-media discovery behind the platform layer and use portable paths for movie lookup/playback.
- [x] Route initialization/shutdown file operations, directory creation/searches, error dialogs and host identity through the platform layer.
- [x] Move local IPv4 enumeration and the console's process-start timestamp behind the platform layer; share adapter ordering and socket-address display code.
- [x] Replace NAT's Windows worker/events with the shared SDL worker and platform synchronization; preserve the negotiation state machine.
- [x] Share dedicated-server console input, command suggestions, profile output and preference-folder log retention through the terminal platform interface.
- [x] Use SDL process creation and platform process handles for slave startup, recovery and bounded shutdown.
- [x] Share executable version metadata and use SDL file timestamps for game-result reporting; preserve fixed-width result payloads.
- [ ] Finish console/debug tools, server dialogs, executable metadata and network-facing platform calls.

**Validation:** the changed browser/dialog, menu, input configuration and WOL sources passed scoped Windows and native Linux compilation. Provider checks verified reference ownership, cleared outputs on failure and matching WOL data layouts on both hosts; Windows checks also verified that neither original online-services nor browser DLL was loaded. Movie/CD sources and native media enumeration compile on both hosts. Linux media support uses `libblkid-dev`, documented in [LinuxBuild.md](LinuxBuild.md). Movie-disc discovery and playback remain untested at runtime. Initialization/shutdown and the changed log/thread sources also passed scoped Windows and Linux compilation. Linux ASan/UBSan checks verified exclusive log-file creation under contention, preservation of existing contents, worker unwinding and completion after an assert-triggered thread exit. A full native Release build compiled the libraries and attempted all game sources, then failed in 26 game translation units with remaining Windows APIs, console/process/event handling, metadata and UI portability errors. These failures are being resolved; executable linking, browser interaction and gameplay validation remain pending.

The latest server-dialog/path, list packing, player/team initialization and SDL desktop-launch changes passed scoped Windows Clang syntax checks with Release definitions. Native validation of these later changes remains pending.

Local IPv4 enumeration now belongs to the network platform interface; Windows retains its existing hostname resolution and Linux retains the agreed network placeholder. Adapter ordering and preferred-adapter selection remain shared game code. The console's process-start timestamp uses platform system information, preserving Windows date/time formatting and reading Linux process-start data from procfs. Socket-address display uses the shared IPv4 representation. The changed adapter enumeration, console commands and Windows network/system-information sources passed scoped Windows Clang syntax checks with Release definitions. Native validation remains pending.

Ubuntu now provides Clang 21 through its default compiler package. The Linux preset uses `clang` and `clang++`; the LLVM 23 minimum applies only to Windows clang-cl. A fresh preset configure identified Clang 21 and compiled its compiler test, but could not finish because copying generated files on the WSL-mounted D: drive returned `Operation not permitted`. The build did not reach game compilation. No WSL mount settings were changed.

The NAT helper's chat endpoint lookup now calls the network platform interface instead of loading INETMIB1.DLL and SNMPAPI.DLL. Windows reads the TCP table through IP Helper, selecting an established connection with the original remote IPv4 address/port criteria. Linux retains the network placeholder. The game-facing WOL provider and NAT negotiation code remain present. A real local TCP connection check verified the returned Windows local address and port, and cleared outputs on failure.

NAT now uses the shared SDL worker, cooperative stop/join and the platform recursive mutex. Pollable events use shared ownership and atomic signaling; queue and result notifications use atomic values. Mutex release happens only after successful acquisition. The NAT helper, firewall wait dialog, WOL join code and game data passed scoped Windows Clang compilation. Linux ASan/UBSan checks covered event lifetime through the actual dynamic-vector queue, reallocation/removal, retained pending events and cross-thread publication. Native game compilation and online negotiation runtime checks remain pending.

Game-data map validation uses a dynamic filename, and host-derived nicknames use platform system information. These changes and the WOL auto-start dialog passed scoped Windows Clang compilation. Build-info parsing reads its existing 32-bit number and stored timestamp without unaligned pointer casts or Windows calendar APIs. The native Linux build-info sanitizer check passed, and Windows date formatting matched the original native API result.

The previous WSL file-permission errors did not recur after the host environment changed: the build directory now appears owned by the WSL user. The complete Linux preset configured successfully with Clang 21, including SDL desktop/GPU/audio and vendored font/image backends. The changed Linux system-information, application handling, network placeholder, SDL worker and desktop-launch sources passed native compilation. A full Release build is in progress through the existing preset; executable linking and runtime validation remain pending.

The first Clang 21 Release attempt found OpenAL Soft 1.25.2's function-effect analysis failure with libstdc++ and was interrupted during combat-library compilation by a server restart. The vendor now pins upstream revision `83ea7236c6f23fc98e2b62eb9c5b3abfd4b2be86`, with a verified archive SHA256. Upstream restricts this analysis to the supported standard library and fixes the MS STL attribute handling. The local warning override and fetched-source mutation have been removed. The build is resuming incrementally, with its log retained outside the repository. [Upstream standard-library fix](https://github.com/kcat/openal-soft/commit/fe0ffd6), [upstream MS STL fix](https://github.com/kcat/openal-soft/commit/83ea723).

The resumed full Release attempt compiled the engine and vendor libraries, including the updated OpenAL sources. Six remaining game-source failures identified Windows console APIs, executable metadata, slave processes, reverse hostname lookup, an unused multimedia header and integer formatting. These now use shared game code and the platform interfaces. Windows scoped compilation passed for all changed game sources and the terminal, process and network implementations. Process integration on Windows and Linux verified launch arguments containing spaces, recovery through a stable process handle, termination and stale-ID rejection. Linux terminal checks verified Enter/Backspace mapping, restoration of terminal settings and plain redirected output. The executable link identified 122 unresolved engine symbols. Archive symbol inspection verified that their definitions were present; missing target dependencies caused Unix archive-order failures. CMake now declares the verified engine dependencies and existing static-library cycles. The complete Linux Release executable and Scripts library linked successfully with Clang 21 on Ubuntu 26.04. The final incremental build returned success with no work remaining, and all executable shared-library dependencies resolved. WSLg initialization testing now uses the existing game installation; runtime gates remain open.

The first WSLg launch reached the Vulkan renderer, then aborted while updating texture thumbnails. GDB traced the failure through `BufferedFileClass::Read` into file closure. A focused ASan reproduction with the existing retail thumbnail file confirmed a write into a freed read buffer: automatic opening invoked `Close()` after that buffer had been allocated. Buffered reads now open before allocating the buffer and close after copying the result. The closed-file, open-file and repeated-read checks passed under ASan/UBSan. Thumbnail timestamp serialization now explicitly uses four bytes; parsing all 1,760 records in the two retail thumbnail files verified that existing layout. The incremental rebuild succeeded. GDB confirmed startup progressed beyond the thumbnail failure to movie playback. Entering the main menu then faulted in font character spacing. A conditional GDB breakpoint captured `0x00690053`, and the core dump showed the UTF-16 bytes for "Single Player" being interpreted as four-byte native characters. Shared chunk string serialization now explicitly reads and writes UTF-16 little endian, including surrogate pairs, without changing the asset format or font renderer. A native ASan/UBSan fixture passed exact-byte output, chunk/micro-chunk round trips with ASCII and non-ASCII characters, and rejection of odd-byte input. Scoped Windows compilation and the native Windows exact-byte/round-trip fixture passed. The updated complete Release build succeeded. GDB observed main-menu activation after control creation, with no recurrence of the font crash. The user confirmed visible menu rendering in WSLg. The X11 run later exited with `BadRROutput`; its call path has not been reproduced. A diagnostic Wayland launch reached the menu, then GDB caught a practice-game crash in `INIClass::Find_Section`. Configuration creation used lowercase `data`, while the file factory read uppercase `DATA` in the case-sensitive preferences directory. Creation and modification-time queries now use the configured file factories. Shared physical path lookup resolves existing filename casing and native absolute paths without moving or migrating files. Native ASan/UBSan checks passed fresh INI creation/save/read, case-insensitive lookup, directory-index refresh, ambiguous-name rejection, empty paths and native absolute paths. Those checks exposed overlapping copies in whitespace trimming; the narrow and wide copies now use overlap-safe moves and the checks pass. The updated executable reached the menu, and the user reached a loading screen. GDB then captured a crash in WeaponClass::Add_Rounds: the data-safe return index was overwritten by an eight-byte copy into a four-byte slot. A focused ASan check reproduced that overread. Data-safe keys, checksum operations and memory copies now use explicit four-byte words. ASan/UBSan checks passed checked and unchecked encryption/decryption for one through eight words, unaligned buffers and the final return slot beside the index. Scoped Windows and Linux checks passed. The subsequent Release rebuild and mission-start check passed. Vulkan enumeration exposes only llvmpipe on this WSL host; hardware performance parity remains untested.

The complete Release rebuild with the data-safe fix succeeded. The user reached the in-game cinematic with working graphics and audio. Loading snapshots showed progress from always-loaded assets to level assets, followed by loader-thread completion. Timings on the same retail archive showed mounted-drive metadata/open operations and cached small reads taking roughly 20–35 times longer under WSL than native Windows; this does not quantify their share of total loading time. After the cinematic, GDB found the gameplay thread repeatedly reading a directory: an empty animation sound filename resolved to `DATA/`, which POSIX open accepted although Windows file opening rejects it. The stream had an error flag and errno `EISDIR`; the audio lock remained held during the retry loop. The POSIX file backend now rejects directory handles after opening. A focused ASan/UBSan check reproduced acceptance before the fix and verified rejection plus normal read/write/EOF/seek behavior afterward. The actual file-factory check also passed empty-filename rejection. The subsequent rebuild and gameplay retest passed as recorded below.

The incremental Release rebuild with directory-handle rejection passed. A direct Windows check confirmed that the same file-opening flags reject the existing retail `Data` directory. The user confirmed that the fixed Linux game progressed past the cinematic into gameplay, with working graphics and audio. Mouse clicks and most keyboard input work; mouse motion and arrow-key behavior remain unverified because this test uses remote desktop. GDB captured Left Arrow reaching the game with input capture active. Live bindings assign Up/Down to movement and Left/Right to turning. The source maps all four arrow scancodes and consumes SDL relative mouse deltas. Local input validation remains pending; no input behavior has been changed on the basis of the remote test.

**Gate:** the main menu, settings, loading screen and single-player startup work through the existing WWUI.

## 5 Validate single player parity

- [ ] Complete this phase.

Run through WSLg with real game data, then test on a native Ubuntu host. Cover texture rendering, movies/audio, cursor and IME, focus/fullscreen changes, mission gameplay, saves/loading and exit. Check case-sensitive asset paths and keep writes in the SDL preference folder. Investigate failures with logs/debugger evidence.

**Gate:** user gameplay feedback confirms parity, with no unexplained assertions or hangs.

## 6 Replace the Linux network placeholders

- [ ] Complete this phase.

Implement socket transport, LAN discovery, hosting/joining and server-control behavior behind the existing interfaces. Preserve packet formats and validate Windows and Linux interoperability. WOL remains the agreed placeholder for the future custom provider.

- [x] Implement Linux socket transport, nonblocking I/O, interface discovery, hostname resolution and TCP endpoint lookup.
- [x] Use the shared LAN, NAT socket and server-control implementations on Windows and Linux; remove their Linux replacement files.
- [x] Preserve four-byte IPv4/checksum fields and reject truncated or undersized datagrams; drain empty datagrams without blocking later packets.
- [ ] Validate LAN discovery and actual game hosting/joining between Windows and Linux.
- [ ] Resolve the remaining bandwidth-probe integration with the future online provider.

**Validation:** native ASan/UBSan checks passed real UDP delivery, socket options, nonblocking/error behavior, address conflicts, truncation handling, interface discovery and established TCP endpoint lookup. Packet-manager checks verified the existing packet header/checksum bytes and payload reconstruction. NAT and encrypted server-control checks verified exact packet bytes and round trips for payload lengths 1-67, plus recovery after empty and undersized datagrams. Shared game/platform sources passed scoped Windows compilation. The full updated Release executable build passed; game-level LAN checks remain pending.

**Gate:** LAN games connect and operate correctly across both hosts.

## 7 Make the Linux deliverable repeatable

- [ ] Complete this phase.

Finish launch settings, dependency deployment and packaging. Validate a clean checkout/build and a packaged launch on Ubuntu, then commit/tag the validated result.
