# Core Module (`engine::core`)

## Overview / Purpose

`engine::core` is the foundational module of the game engine. It delivers the core architectural infrastructure upon which all other engine subsystems and game logic are built.

The core responsibilities of this module include:
- **Entity Component System (ECS):** A lightweight, cache-friendly, generational 32-bit ECS implementation (`Coordinator`, `EntityManager`, `ComponentManager`, `SystemManager`) utilizing sparse arrays and bitset signatures.
- **Event Bus:** A type-safe, decoupled publish/subscribe event dispatcher (`EventBus`, `Subscription`) with RAII subscription handles and re-entrancy protection.
- **Logging Subsystem:** A centralized diagnostics logging service (`Logger`) built on the Service Locator pattern, providing four severity levels (`DEBUG`, `INFO`, `WARN`, `ERROR`) and file/console stream multiplexing. See [Logger Architecture](logger.md) for detailed guidelines.
- **Metrics & Telemetry:** Performance and network telemetry tracking (`Metrics`, `MetricsSnapshot`) monitoring frame budgets, tick durations, packet round-trip times (RTT), jitter, and bandwidth.
- **Configuration & Settings:** File-based key-value configuration parsing (`ConfigParser`, `EngineSettings`) for game configurations, control maps, and accessibility flags.
- **Standard Systems & Utilities:** Shared general-purpose systems including `MovementSystem` (integrating velocity into position), `InterpolationSystem` (client-side render smoothing via `InterpolationClock`), `DestroySystem` (deferred entity cleanup), and `TimedQueue` (scheduled execution).

---

## Utility & Audience

### Architectural Benefits
- **Zero-Coupling Foundation:** `engine::core` contains no graphics, audio, or networking code. It can be compiled in any environment, including headless servers, command-line test runners, and graphical clients.
- **Deterministic Simulation:** Encapsulates the core math, ECS state, and fixed-timestep update logic required for multiplayer synchronization.
- **Inversion of Control:** Components such as `Logger` and `EventBus` permit loose coupling across engine modules without static initialization order hazards.

### Target Audience
- **Internal Engine Modules:** All other engine modules (`engine::render`, `engine::audio`, `engine::input`, `engine::physics`, `engine::net`, `engine::assets`, `engine::tools`) depend directly on `engine::core`.
- **Server Applications (`r-type_server`):** Utilizes the ECS, `MovementSystem`, `EventBus`, `Metrics`, and `Logger` for headless authoritative world simulation.
- **Client Applications (`r-type_client`):** Utilizes the ECS, client-side interpolation, configuration parsers, and event listeners.

---

## Dependencies

- **Internal Dependencies:** None. (This is the root dependency for the entire engine.)
- **External Dependencies:** C++20 Standard Library (`<vector>`, `<unordered_map>`, `<bitset>`, `<any>`, `<chrono>`, `<filesystem>`, `<memory>`, `<iostream>`). No third-party packages required.

---

## Usage Guide

### 1. Initializing the ECS Coordinator and Registering Components

The `Coordinator` orchestrates entities, components, and systems:

```cpp
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/Components.hpp"

// Define custom component types
struct Health {
    int current = 100;
    int max = 100;
};

int main() {
    engine::core::Coordinator coordinator;

    // Register built-in and user-defined components
    coordinator.registerComponent<engine::core::Position>();
    coordinator.registerComponent<engine::core::Velocity>();
    coordinator.registerComponent<Health>();

    // Create an entity and attach components
    engine::core::Entity player = coordinator.createEntity();
    coordinator.addComponent(player, engine::core::Position{100.0f, 200.0f});
    coordinator.addComponent(player, engine::core::Velocity{50.0f, 0.0f});
    coordinator.addComponent(player, Health{100, 100});

    // Query entity state
    if (coordinator.hasComponent<engine::core::Position>(player)) {
        auto& pos = coordinator.getComponent<engine::core::Position>(player);
        pos.x += 10.0f;
    }

    return 0;
}
```

### 2. Subscribing to and Publishing Events via `EventBus`

The `EventBus` provides safe, typed message passing between decoupled subsystems:

```cpp
#include <iostream>
#include "engine/core/EventBus.hpp"

struct PlayerDiedEvent {
    uint32_t playerId;
    std::string reason;
};

void runEventBusExample() {
    engine::core::EventBus bus;

    // Subscribe with an RAII handle (unsubscribes automatically when destroyed)
    engine::core::Subscription sub = bus.subscribe<PlayerDiedEvent>(
        [](const PlayerDiedEvent& event) {
            std::cout << "Player " << event.playerId << " died: " << event.reason << std::endl;
        }
    );

    // Publish event
    bus.publish(PlayerDiedEvent{1, "Collision with asteroid"});
}
```

### 3. Setting Up the Global Logger

Initialize and configure logging via the Service Locator pattern:

```cpp
#include <iostream>
#include <memory>
#include "engine/core/Logger.hpp"

void initLogging(bool debugMode) {
    auto logger = std::make_shared<engine::core::Logger>(std::cout);
    if (debugMode) {
        logger->setVerbose(true); // Enables DEBUG severity
    }
    engine::core::Logger::setDefaultLogger(logger);

    engine::core::Logger::defaultLogger().info("Engine core initialized successfully.");
}
```

For detailed logging practices and rules, refer to [Logger Architecture](logger.md).
