# Build Instructions

This guide provides step-by-step instructions for compiling the **R-Type** project from source on both **Linux** and **Windows**, as well as instructions for cross-compiling Windows binaries from Linux using MinGW.

---

## 1. Prerequisites and Toolchains

The project is built with **C++20** and managed with **CMake** (3.21 or newer). Dependencies are resolved through [vcpkg](https://vcpkg.io/) in **Manifest Mode** (`vcpkg.json`), which downloads and compiles the exact required versions of **Asio** and **SFML 3.0.2** (Graphics, Window, System, Audio).

### Linux (Ubuntu / Debian)

Install the standard C++20 compiler (`g++` or `clang++`), CMake, Ninja, Git, and the required development libraries for SFML:

```bash
sudo apt-get update
sudo apt-get install -y \
    git cmake ninja-build curl zip unzip tar \
    pkg-config build-essential \
    libx11-dev libxcursor-dev libxi-dev \
    libgl1-mesa-dev libfontconfig1-dev \
    libudev-dev
```

Verify your toolchain versions:

```bash
git --version
cmake --version    # Must be >= 3.21
g++ --version      # Must support C++20 (GCC 11+)
ninja --version
```

### Windows (MSVC)

1. **Git for Windows**: [Download Git](https://git-scm.com/downloads)
2. **Visual Studio 2022 Community** (or newer): [Download Visual Studio](https://visualstudio.microsoft.com/downloads/)
3. In the **Visual Studio Installer**, enable the **Desktop development with C++** workload. Ensure the following individual components are selected:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10 or 11 SDK
   - C++ CMake tools for Windows
   - Ninja build system (recommended)

Verify your environment in PowerShell:

```powershell
git --version
cmake --version
```

---

## 2. Cloning the Repository

The repository includes `vcpkg` as a Git submodule. Clone the project recursively:

```bash
git clone --recursive https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-1.git r-type
cd r-type
```

If you previously cloned without the `--recursive` flag, initialize and update the submodule manually:

```bash
git submodule update --init --recursive
```

---

## 3. Bootstrapping Vcpkg

Before running CMake, the local `vcpkg` submodule must be bootstrapped once to generate the package manager executable:

### On Linux
```bash
./vcpkg/bootstrap-vcpkg.sh
```

### On Windows (PowerShell / Command Prompt)
```cmd
.\vcpkg\bootstrap-vcpkg.bat
```

> **Note:** Bootstrapping only compiles the `vcpkg` binary tool itself. It does not download project dependencies yet.

---

## 4. Configuring the Project with CMake

CMake integrates with `vcpkg` through its toolchain file. During configuration, `vcpkg` reads `vcpkg.json`, downloads all required libraries (Asio, SFML 3.0.2, and their dependencies), and builds them into `build/vcpkg_installed/`.

### Native Linux Build

```bash
cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

### Native Windows Build (PowerShell)

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Cross-Compilation (Linux to Windows via MinGW)

To cross-compile 64-bit Windows executables on a Linux host using MinGW and Ninja:

```bash
cmake -S . -B build-windows -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$PWD/mingw-toolchain.cmake"
```

Both Windows executables automatically configure `_WIN32_WINNT=0x0601` (Windows 7 target) for Asio networking compatibility and link against `ws2_32` and `bcrypt`.

---

## 5. Compiling the Project

Once configuration completes, compile all targets in parallel:

### Linux / MinGW (Ninja or Make)
```bash
cmake --build build --parallel
```

### Windows (Visual Studio multi-config)
```powershell
cmake --build build --config Release --parallel
```

### Asset Synchronization Target

The build system defines a custom CMake target called `rtype_assets`. Every time the client binary is compiled, CMake automatically synchronizes the root `assets/` directory (containing spritesheets, music, sound effects, and fonts) into the executable's output directory (`$<TARGET_FILE_DIR:r-type_client>/assets`). This ensures the client can locate media assets regardless of the current working directory.

### Building the Engine Alone

The engine in `engine/` is a standalone CMake project with no dependency on the game. It can be built and installed on its own:

```bash
cmake -S engine -B build-engine -G Ninja
cmake --build build-engine
cmake --install build-engine --prefix install
```

A game can then consume it like any third-party package:

```cmake
find_package(Engine 0.1 CONFIG REQUIRED)
target_link_libraries(my_game PRIVATE engine::core)
```

`games/rtype/CMakeLists.txt` follows this pattern: built from the repository root it uses the `engine::core` target from `add_subdirectory(engine)`, and built alone it calls `find_package(Engine)` (pass the install prefix in `CMAKE_PREFIX_PATH`).

---

## 6. Build Artifacts

After a successful build, the binaries are located in:

| Platform / Toolchain | Server Executable | Client Executable |
|---|---|---|
| **Linux (Native)** | `build/r-type_server` | `build/r-type_client` |
| **Windows (MSVC)** | `build/Release/r-type_server.exe` | `build/Release/r-type_client.exe` |
| **Windows (MinGW)** | `build-windows/r-type_server.exe` | `build-windows/r-type_client.exe` |

For instructions on executing and configuring both server and client, refer to [run_instructions.md](run_instructions.md).

---

## 7. Troubleshooting

- **Missing Submodules:** If CMake fails with `vcpkg.cmake does not exist`, ensure the submodule is populated by running `git submodule update --init --recursive`.
- **Missing Linux X11/GL headers:** If SFML compilation fails during vcpkg installation, ensure `libx11-dev`, `libgl1-mesa-dev`, and `libudev-dev` are installed.
- **Cache Invalidation:** To perform a clean rebuild, delete the `build/` directory and re-run the CMake configure step.
