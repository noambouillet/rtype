# R-Type

This project is an online multiplayer 2D side-scrolling shoot'em up game inspired by the 1987 arcade classic **R-Type**, developed in **C++20** as a 3rd-year Epitech project.

The project features a custom cross-platform game engine built from scratch around an Entity Component System (ECS), an authoritative headless UDP server, a graphical SFML client with client-side prediction, and a strong focus on accessibility.

---

## Quick Start (TL;DR)

Get the game running on Linux in four commands:

```bash
# 1. Clone repository with submodules
git clone --recursive https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-1.git r-type
cd r-type

# 2. Bootstrap vcpkg & build
./vcpkg/bootstrap-vcpkg.sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# 3. Start authoritative server (Terminal 1)
./build/r-type_server

# 4. Start graphical client (Terminal 2)
./build/r-type_client
```

> **Windows (PowerShell):** Replace `./vcpkg/bootstrap-vcpkg.sh` with `.\vcpkg\bootstrap-vcpkg.bat`, and run `cmake --build build --config Release --parallel`. Binaries will be generated under `build/Release/`.

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

Dependencies are automatically resolved and compiled via [vcpkg](https://vcpkg.io/) in **Manifest Mode** (`vcpkg.json`). You do not need to install Asio or SFML system-wide.

### System Requirements

- **C++ Standard:** C++20 compliant compiler (GCC 11+, Clang 14+, or MSVC 2022).
- **Build System:** CMake >= 3.21 and Ninja (or Make).
- **Version Control:** Git (with submodules enabled).

### Linux Packages (Ubuntu / Debian)

Install development libraries required for building SFML:

```bash
sudo apt-get update && sudo apt-get install -y \
    git cmake ninja-build curl zip unzip tar \
    pkg-config build-essential \
    libx11-dev libxcursor-dev libxi-dev \
    libgl1-mesa-dev libfontconfig1-dev \
    libudev-dev
```

### Managed Third-Party Libraries

- **Asio:** Asynchronous I/O and UDP network transport.
- **SFML 3.0.2:** Hardware-accelerated 2D graphics, audio synthesis, and window management.

---

## Build Instructions

### 1. Initialize Submodules

If you cloned without `--recursive`, initialize the `vcpkg` submodule first:

```bash
git submodule update --init --recursive
```

### 2. Bootstrap Vcpkg

Compile the package manager executable once:

- **Linux:** `./vcpkg/bootstrap-vcpkg.sh`
- **Windows:** `.\vcpkg\bootstrap-vcpkg.bat`

### 3. Configure with CMake

- **Linux:**
  ```bash
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
  ```
- **Windows (MSVC):**
  ```powershell
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
  ```
- **Cross-Compilation (Linux to Windows via MinGW):**
  ```bash
  cmake -S . -B build-windows -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
      -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic \
      -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$PWD/mingw-toolchain.cmake"
  ```

### 4. Compile

```bash
cmake --build build --parallel
```

> **Asset Synchronization:** The build system automatically copies the `assets/` directory (sprites, sounds, music, fonts) into the client binary output folder so the game can be launched from any working directory.

For comprehensive toolchain details and troubleshooting, see the [**Full Build Guide**](docs/build_instructions.md).

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
├── src/
│   ├── client/                 # SFML graphical client and presentation systems
│   │   ├── app/                # Client application lifecycle and CLI parsing
│   │   ├── network/            # Asynchronous UDP client transport
│   │   ├── SFML/               # SFML 3 graphics, audio backends, and keybindings
│   │   └── view/               # Client view ECS, sprite catalog, and prediction
│   ├── game/                   # Shared game types, button bitmasks, and constants
│   ├── game_engine/            # Custom ECS core library (rtype_engine)
│   │   ├── Audio/              # Abstract audio backend interface
│   │   ├── Component/          # Sparse set ComponentArray and ComponentManager
│   │   ├── Coordinator/        # ECS facade coordinating entities and managers
│   │   ├── Entity/             # 32-bit generational Entity and EntityManager
│   │   ├── Graphics/           # Abstract graphics backend interface
│   │   └── System/             # SystemManager and core systems (movement, collision...)
│   ├── server/                 # Authoritative headless game server (r-type_server)
│   │   ├── ecs/                # Server gameplay systems (weapon, spawn, combat...)
│   │   ├── game/               # Server GameWorld, tick loop, prefabs, and events
│   │   └── network/            # Asynchronous UDP server transport and sessions
│   └── utils/                  # Shared protocol encoders/decoders and logging
├── vcpkg/                      # Vcpkg package manager (Git submodule)
├── website/                    # Doxygen documentation website configuration
├── CMakeLists.txt              # Root CMake build definition
├── LICENSE                     # MIT License
├── mingw-toolchain.cmake       # MinGW cross-compilation toolchain file
├── settings.cfg                # Client runtime settings (keybinds, controller, flash)
└── vcpkg.json                  # Manifest specifying external dependencies (Asio, SFML)
```

---

## Documentation Index

For in-depth architectural and technical documentation:

- [**Build Instructions**](docs/build_instructions.md): Prerequisites, toolchains, vcpkg bootstrap, and CMake build steps.
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
