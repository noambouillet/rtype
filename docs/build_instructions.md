# Build Instructions

This guide provides instructions for compiling the **R-Type** project from source on **Linux** and **Windows**, as well as cross-compiling for Windows from Linux.

---

## 1. Quick Start (Automated One-Step Build)

Automated scripts are provided in the repository root to detect your operating system, check and install any missing system dependencies (compiler, CMake, SFML libraries), initialize the `vcpkg` git submodule, bootstrap `vcpkg`, and compile the project in parallel.

### Linux / macOS
```bash
# Standard build (prompts before installing missing dependencies)
./build.sh

# Non-interactive / CI build (auto-accepts package installations)
./build.sh -y

# Clean rebuild (deletes build directory before building)
./build.sh --clean

# Cross-compile Windows 64-bit binaries (.exe) via MinGW on Linux
./build.sh -w

# Specify a custom build directory
./build.sh -b custom_build
```

**Available Options:**
| Flag | Description |
|---|---|
| `-w`, `--windows` | Cross-compile 64-bit Windows binaries (.exe) using MinGW (defaults to `build-windows/`) |
| `-b`, `--build-dir <DIR>` | Set a custom build output directory (default: `build` or `build-windows`) |
| `-c`, `--clean` | Wipe the target build directory before configuring and compiling |
| `-y`, `--yes` | Automatically confirm package installation with the system package manager without prompting |
| `-h`, `--help` | Display the script usage help |

### Windows (PowerShell)
```powershell
# Standard build with MSVC (prompts before downloading missing tools)
.\build.ps1

# Non-interactive build (auto-downloads missing Git, CMake, or MSVC)
.\build.ps1 -Yes

# Clean rebuild
.\build.ps1 -Clean

# Custom build directory
.\build.ps1 -BuildDir "custom_build"
```

**Available Options:**
| Parameter | Description |
|---|---|
| `-Yes`, `-y` | Automatically download and install missing tools without prompting |
| `-BuildDir <DIR>`, `-b` | Set a custom build directory (default: `build`) |
| `-Clean`, `-c` | Wipe the target build directory before configuring and compiling |
| `-Help`, `-h` | Display the help documentation |

---

## 2. Build Output and Binaries

Once the build finishes successfully, executables and media assets are placed in the output directory:

| Target Platform / Host | Server Executable | Client Executable |
|---|---|---|
| **Linux (Native)** | `build/r-type_server` | `build/r-type_client` |
| **Windows (Native MSVC)** | `build/Release/r-type_server.exe` | `build/Release/r-type_client.exe` |
| **Windows (Cross-compiled via MinGW)** | `build-windows/r-type_server.exe` | `build-windows/r-type_client.exe` |

> **Asset Synchronization:** The root `assets/` folder (textures, fonts, music, audio effects) is automatically copied into the client's output folder on each compile.

### Running the Game

```bash
# Start the dedicated server (specifying port, e.g. 4242)
./build/r-type_server 4242

# Start the graphical client (connecting to server IP and port)
./build/r-type_client 127.0.0.1 4242
```

For more details on server/client CLI parameters and runtime settings, refer to [**Run Instructions**](run_instructions.md).

---

## 3. Building the Engine Alone (Standalone)

To strictly enforce separation of concerns, the engine in `engine/` is a standalone CMake project with no dependency on the game. It can be built and installed on its own:

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

`games/rtype/CMakeLists.txt` follows this pattern: built from the repository root it uses the `engine::core` target from `add_subdirectory(engine)`. If built entirely standalone, it resolves via `find_package(Engine)` (by passing the install prefix in `CMAKE_PREFIX_PATH`).

---

## 4. Manual Build (Optional / Advanced)

If you prefer to configure and build manually without using `build.sh` or `build.ps1`:

### Step 1: Install System Dependencies

- **Ubuntu / Debian (Native Linux):**
  ```bash
  sudo apt-get update && sudo apt-get install -y \
      git cmake ninja-build curl zip unzip tar pkg-config build-essential \
      libx11-dev libxcursor-dev libxi-dev libgl1-mesa-dev libfontconfig1-dev libudev-dev
  ```
- **Ubuntu / Debian (Cross-compilation for Windows):**
  ```bash
  sudo apt-get update && sudo apt-get install -y \
      git cmake ninja-build curl zip unzip tar pkg-config build-essential mingw-w64
  ```
- **Fedora / RHEL:**
  ```bash
  sudo dnf install -y \
      git cmake ninja-build curl zip unzip tar pkgconf-pkg-config gcc-c++ make \
      libX11-devel libXcursor-devel libXi-devel mesa-libGL-devel fontconfig-devel systemd-devel
  ```
- **Arch Linux:**
  ```bash
  sudo pacman -Sy --noconfirm \
      git cmake ninja curl zip unzip tar pkgconf base-devel \
      libx11 libxcursor libxi mesa fontconfig systemd
  ```
- **Windows:** Visual Studio 2022 Community with the **Desktop development with C++** workload (including MSVC v143 toolset, Windows SDK, and CMake tools).

### Step 2: Initialize Vcpkg Submodule

```bash
git submodule update --init --recursive
./vcpkg/bootstrap-vcpkg.sh       # On Windows: .\vcpkg\bootstrap-vcpkg.bat
```

### Step 3: Configure and Compile with CMake

#### Native Linux Build
```bash
cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build --parallel
```

#### Native Windows Build (PowerShell / Visual Studio)
```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release --parallel
```

#### Windows Cross-Compilation on Linux (MinGW)
```bash
cmake -S . -B build-windows -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic \
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$PWD/mingw-toolchain.cmake"

cmake --build build-windows --parallel
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
find_package(Engine 0.1 CONFIG REQUIRED COMPONENTS core physics net)
target_link_libraries(my_game PRIVATE engine::core engine::physics engine::net)
```

`games/rtype/CMakeLists.txt` follows this pattern: built from the repository root it uses the `engine::*` targets from `add_subdirectory(engine)`, and built alone it calls `find_package(Engine)` (pass the install prefix in `CMAKE_PREFIX_PATH`).

The engine is split into modules selected with `ENGINE_BUILD_*` options, and the root build selects them with `RTYPE_BUILD_SERVER` / `RTYPE_BUILD_CLIENT`. See [Engine Modules](engine_modules.md).

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
