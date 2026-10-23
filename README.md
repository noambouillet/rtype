# R-Type

This project is an online multiplayer 2D side-scrolling shoot'em up game inspired by the 1987 arcade classic **R-Type**, developed in **C++20** as a 3rd-year Epitech project.

The project features a custom cross-platform game engine built from scratch around an Entity Component System (ECS), an authoritative headless UDP server, a graphical SFML client with client-side prediction, and a strong focus on accessibility.

---

## Quick Start (TL;DR)

Get the game running on Linux in three commands:

```bash
# 1. Clone repository with submodules if you don't cloned it already
git clone --recursive https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-1.git r-type
cd r-type

# 2. Build with automated script (handles dependencies, vcpkg & parallel compilation)
./build.sh

# 3. Start authoritative server (Terminal 1)
./build/r-type_server

# 4. Start graphical client (Terminal 2)
./build/r-type_client
```

> **Windows (PowerShell):** Run `.\build.ps1` instead of `./build.sh`. Binaries will be generated under `build/Release/`.

---

## Supported Platforms

| Platform | Support Level | Toolchain |
|---|---|---|
| **Linux** | Native (Ubuntu 22.04+, Debian 12+, Fedora, Arch) | GCC 11+ / Clang 14+ |
| **Windows** | Native (Windows 10 / 11) | MSVC 2022 (v143) / Clang-cl |
| **Windows Cross-Compile** | MinGW-w64 compiled from Linux | MinGW GCC + Ninja |

> **Cross-Play:** Full network interoperability is supported. Linux and Windows clients can seamlessly connect to the same game server.

---

## Dependencies & Requirements

System packages, toolchains, and third-party libraries are handled automatically by the automated build scripts (`build.sh` / `build.ps1`).

### System Requirements

- **C++ Standard:** C++20 compliant compiler (GCC 11+, Clang 14+, or MSVC 2022).
- **Build System:** CMake >= 3.21 and Ninja (or Make).
- **Version Control:** Git (with submodules enabled).

> **Manual Dependency Setup:** The automated scripts will check and install missing tools and graphic/audio libraries for your Linux distribution (APT, DNF, Pacman) or Windows (via Winget). If you prefer installing packages manually, see the [**Manual Package Installation Guide**](docs/build_instructions.md#step-1-install-system-dependencies).

### Managed Third-Party Libraries

- **Asio:** Asynchronous I/O and UDP network transport.
- **SFML 3.0.2:** Hardware-accelerated 2D graphics, audio synthesis, and window management.

---

## Build Instructions

Automated scripts are provided in the repository root to detect your operating system, check and install any missing system dependencies (compiler, CMake, SFML libraries), initialize the `vcpkg` git submodule, bootstrap `vcpkg`, and compile the project in parallel.

### 1. Automated Build (One-Step Build)

#### Linux / macOS
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

#### Windows (PowerShell)
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

### 2. Build Output and Binaries

Once the build finishes successfully, executables and media assets are placed in the output directory:

| Target Platform / Host | Server Executable | Client Executable |
|---|---|---|
| **Linux (Native)** | `build/r-type_server` | `build/r-type_client` |
| **Windows (Native MSVC)** | `build/Release/r-type_server.exe` | `build/Release/r-type_client.exe` |
| **Windows (Cross-compiled via MinGW)** | `build-windows/r-type_server.exe` | `build-windows/r-type_client.exe` |

> **Asset Synchronization:** The root `assets/` folder (textures, fonts, music, audio effects) is automatically copied into the client's output folder on each compile.

### 3. Manual Build (Optional / Advanced)

If you prefer to configure, bootstrap `vcpkg`, and compile manually without using `build.sh` or `build.ps1`, refer to the detailed step-by-step instructions in the [**Manual Build Guide**](docs/build_instructions.md#3-manual-build-optional--advanced).

---

## Usage Instructions

### Running the Server

The server is a headless, authoritative simulation running at a fixed 60 Hz tick rate.

```bash
./build/r-type_server [port] [--log-file=path] [-h]
```

- **Default Launch (Port 6767):**
  ```bash
  ./build/r-type_server
  ```
- **Custom Port with File Logging:**
  ```bash
  ./build/r-type_server 4242 --log-file=server.log
  ```

### Running the Client

The client is a graphical SFML 3 application with client-side weapon prediction and parallax rendering.

```bash
./build/r-type_client [host] [port] [--log-file=path] [-h]
```

- **Connect to Localhost (Default Port 6767):**
  ```bash
  ./build/r-type_client
  ```
- **Connect to Remote Server:**
  ```bash
  ./build/r-type_client 192.168.1.50 6767
  ```

### Controls & Configuration

Controls and accessibility settings can be customized in `settings.cfg` (generated automatically on first run):

| Action | Default Keyboard Key | Gamepad (Controller) |
|---|---|---|
| **Move Up / Down / Left / Right** | `Z` / `S` / `Q` / `D` (or Arrows) | Left Analog Stick / D-Pad |
| **Fire Weapon** | `Space` | Button 0 (A / Cross) |
| **Toggle Auto-Fire** | `X` | Button 1 (B / Circle) |
- **Controller Deadzone:** Adjust `joystick_deadzone = 30` to counter stick drift.
- **Visual Accessibility:** Set `screen_flash = false` to suppress bright explosion flashes for photosensitive safety.

For multiple clients and local testing workflows, see the [**Full Run Guide**](docs/run_instructions.md).

---

## Key Features

- **Custom Entity Component System (ECS):** Composition-based architecture (`rtype_engine`) with 32-bit generational entities and cache-friendly sparse sets.
- **Authoritative Headless Server:** Simulates physics, collisions, and waves at 60 Hz; broadcasts binary world snapshots to up to 4 players.
- **Asynchronous UDP Protocol:** Minimal overhead, binary serialization, sequencing, and packet validation using Asio.
- **Client-Side Prediction:** Instant weapon feedback locally to hide network latency.
- **Accessibility by Design (GAG / XAG):** Rebindable keyboard and controller inputs, analog deadzones, auto-fire mode to reduce motor fatigue, and screen flash suppression.

---

## Repository Structure

```text
r-type/
├── assets/                     # Spritesheets, fonts, sound effects, and music
├── docs/                       # Technical guides, protocol RFCs, and engine studies
│   ├── accessibility_study.md  # Motor & visual accessibility technical study
│   ├── architecture.md         # Layered subsystem and engine systems overview
│   ├── build_instructions.md   # Step-by-step compilation guide (Linux & Windows)
│   ├── coding-style.md         # C++20 conventions and documentation standards
│   ├── contributing.md         # Conventional commits workflow and git hooks
│   ├── ecs_guide.md            # Detailed ECS guide (Entities, Components, Systems)
│   ├── RFC.md                  # UDP network binary protocol specification
│   ├── R-TYPE.md               # Original game design and assignment reference
│   └── run_instructions.md     # Server/client execution and CLI arguments
├── engine/                     # Standalone engine project (find_package(Engine))
│   ├── cmake/                  # EngineConfig.cmake.in package template
│   ├── include/engine/         # Public headers
│   │   ├── audio/              # Abstract audio backend interface
│   │   ├── core/               # Errors, logger, config parser, engine settings
│   │   │   ├── ecs/            # Entities, sparse set components, systems, Coordinator
│   │   │   └── systems/        # Generic systems (movement, collision, render...)
│   │   ├── net/                # Abstract client/server transport interfaces
│   │   └── render/             # Abstract graphics backend interface
│   ├── src/                    # Private engine implementation
│   ├── CMakeLists.txt          # Engine build, install, and export rules (engine::core)
│   └── vcpkg.json              # Engine-only dependencies
├── games/
│   └── rtype/                  # R-Type game, links engine::core like a third-party lib
│       ├── src/client/         # SFML graphical client and presentation systems
│       ├── src/game/           # Shared game types, button bitmasks, and constants
│       ├── src/server/         # Authoritative headless game server (r-type_server)
│       ├── src/utils/          # Shared protocol encoders/decoders and game errors
│       └── CMakeLists.txt      # Game targets (r-type_server, r-type_client)
├── vcpkg/                      # Vcpkg package manager (Git submodule)
├── website/                    # Doxygen documentation website configuration
├── build.sh                    # Automated setup & build script (Linux / macOS)
├── build.ps1                   # Automated setup & build script (Windows PowerShell)
├── LICENSE                     # MIT License
├── CMakeLists.txt              # Root build: adds engine/ and games/rtype/
├── mingw-toolchain.cmake       # MinGW cross-compilation toolchain file
├── settings.cfg                # Client runtime settings (keybinds, controller, flash)
└── vcpkg.json                  # Manifest specifying external dependencies (Asio, SFML)
```

---

## Documentation Index

For in-depth architectural and technical documentation:

- [**Build Instructions**](docs/build_instructions.md): Automated one-step setup and build (`build.sh` / `build.ps1`), toolchains, and CMake manual compilation steps.
- [**Run Instructions**](docs/run_instructions.md): Server and client CLI flags, `settings.cfg`, and multi-client testing.
- [**Architecture & Systems Overview**](docs/architecture.md): Subsystems breakdown (Engine vs Client vs Server), Render, Physics, and Network systems.
- [**Entity Component System (ECS) Guide**](docs/ecs_guide.md): In-depth look at our ECS paradigm, generational IDs, and sparse sets.
- [**Accessibility Technical Study**](docs/accessibility_study.md): Study of implemented accessibility features and GAG/XAG compliance.
- [**UDP Network Protocol (RFC)**](docs/RFC.md): Packet wire format, headers, sequence validation, and session states.
- [**R-Type Gameplay Reference**](docs/R-TYPE.md): Overview of arcade mechanics, enemy types, and weapons.
- [**Coding Style Guide**](docs/coding-style.md): C++ code conventions, naming rules, and documentation standards.
- [**Contributing Guide**](docs/contributing.md): Branching strategy, Conventional Commits format, and git hooks.
- [**Documentation Website Guide**](website/README.md): Instructions to generate the complete Doxygen HTML documentation.

---

## Collaborators

- **Noam Bouillet** ([@noambouillet](https://github.com/noambouillet)) — <noam.bouillet@epitech.eu>
- **Keryan Pollet Druelle** ([@Keryan-pollet](https://github.com/Keryan-pollet)) — <keryan.pollet-druelle@epitech.eu>
- **Logan Capoen** ([@logan-capoen](https://github.com/logan-capoen)) — <logan.capoen@epitech.eu>

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.
