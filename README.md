# R-Type

This project is an online multiplayer 2D side-scrolling shoot'em up game inspired by the 1987 arcade classic **R-Type**, developed in **C++20** as a 3rd-year Epitech project.

---

## Project Goals

The objective of this project is to build a complete, cross-platform multiplayer game and game engine from scratch without relying on off-the-shelf commercial game engines (like Unity or Unreal).

Key goals include:
- **Custom Game Engine:** Designing and building a lightweight, high-performance **Entity Component System (ECS)** to cleanly manage entities, state, and behaviors.
- **Authoritative Multiplayer Architecture:** Developing a dedicated headless server that runs authoritative gameplay physics and game state at a fixed 60 Hz tick rate.
- **UDP Network Protocol:** Implementing an asynchronous binary wire protocol over non-blocking UDP sockets using **Asio** to synchronize game state with minimal latency.
- **Graphical Client & Prediction:** Building an interactive client using **SFML** with local client-side prediction, parallax scrolling, visual effects, and audio synthesis.
- **Modern Accessibility:** Ensuring the game is playable by everyone in many ways, specifically designed to help people with physical and motor, audio and visual, and mental and cognitive disabilities.

---

## Key Concepts Addressed

- **Entity Component System (ECS):** Composition-based architecture replacing rigid OOP inheritance trees. Uses 32-bit generational entity handles and sparse set component storage for fast, cache-friendly lookups.
- **Client-Server Networking:** Authoritative server architecture where clients send input sampled at 60 Hz and the server broadcasts world snapshots and discrete combat events over UDP.
- **Client-Side Prediction:** Instant local feedback on weapon firing to mask network latency.
- **Hardware Abstraction:** Abstract graphics and audio backend interfaces (`IGraphicsBackend`, `IAudioBackend`), keeping the core game engine independent of SFML.
- **Game Accessibility:** Compliance with international accessibility principles (GAG / XAG) across motor, fatigue, and visual/cognitive dimensions.
- **Modern C++20 & DevOps:** Standardized with C++20 concepts, CMake build targets, and automated dependency management via **vcpkg** (Manifest Mode).

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
├── mingw-toolchain.cmake       # MinGW cross-compilation toolchain file
├── settings.cfg                # Client runtime settings (keybinds, controller, flash)
└── vcpkg.json                  # Manifest specifying external dependencies (Asio, SFML)
```

---

## Collaborators

This project is developed by:

- **Noam Bouillet** ([@noambouillet](https://github.com/noambouillet)) — <noam.bouillet@epitech.eu>
- **Keryan Pollet Druelle** ([@Keryan-pollet](https://github.com/Keryan-pollet)) — <keryan.pollet-druelle@epitech.eu>
- **Logan Capoen** ([@logan-capoen](https://github.com/logan-capoen)) — <logan.capoen@epitech.eu>

---

## Documentation Index

For detailed instructions and technical documentation, refer to the guides below:

- [**Build Instructions**](docs/build_instructions.md): Prerequisites, toolchains, vcpkg bootstrap, and CMake build steps for Linux and Windows.
- [**Run Instructions**](docs/run_instructions.md): How to launch the server and client, CLI parameters, `settings.cfg`, and testing multiple clients.
- [**Architecture & Systems Overview**](docs/architecture.md): High-level system breakdown (Engine vs Client vs Server) and details on Network, Render, and Physics systems.
- [**Entity Component System (ECS) Guide**](docs/ecs_guide.md): In-depth look at our ECS paradigm, generational IDs, sparse set memory, and system signatures.
- [**Accessibility Technical Study**](docs/accessibility_study.md): Study of implemented accessibility features (remapping, deadzones, auto-fire, flash suppression) and GAG/XAG compliance.
- [**UDP Network Protocol (RFC)**](docs/RFC.md): Wire format, packet headers, sequence validation, session management, and payload byte layouts.
- [**R-Type Gameplay Reference**](docs/R-TYPE.md): Overview of arcade mechanics, enemy types, weapons, and assignment specifications.
- [**Coding Style Guide**](docs/coding-style.md): C++ code conventions, naming rules, and documentation standards.
- [**Contributing Guide**](docs/contributing.md): Branching strategy, Conventional Commits format, and git hook setup.
- [**Documentation Website Guide**](website/README.md): Instructions to generate and browse the complete Doxygen HTML documentation site.
