# R-Type documentation

R-Type is a C++20 multiplayer shoot-'em-up project. This site contains the
project's Markdown guides and generated documentation for the C++ source.
The server runs authoritative gameplay and the graphical client presents
replicated world snapshots through a view ECS.

## Guides

- [Build instructions](../docs/build_instructions.md): dependencies, toolchains, vcpkg setup, and compilation on Linux and Windows.
- [Run instructions](../docs/run_instructions.md): binary execution, command-line arguments, `settings.cfg`, and multiplayer workflows.
- [Architecture & systems](../docs/architecture.md): engine layers, network authoritative loop, rendering pipeline, and physics/movement.
- [Entity Component System guide](../docs/ecs_guide.md): ECS paradigm, generational entity IDs, sparse sets, and system signatures.
- [Accessibility technical study](../docs/accessibility_study.md): motor and visual accessibility features, comparative analysis, and standards compliance.
- [UDP protocol](../docs/RFC.md): network wire protocol, session handshakes, sequence numbers, and packet specifications.
- [Metrics](../docs/metrics.md): performance and network metrics, the in-game overlay (F3), server reports, and how to measure under bad network conditions.
- [Coding style](../docs/coding-style.md): C++ conventions and `///` comments.
- [Contributing](../docs/contributing.md): project workflow and conventional commits.
- [R-Type gameplay reference](../docs/R-TYPE.md): original game mechanics and assignment requirements.

## API

Use the **Classes**, **Namespaces**, and **Files** sections in the navigation
to browse the API generated from C++ declarations and `///` comments.
