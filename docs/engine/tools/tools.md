# Tools Module (`engine::tools`)

## Overview / Purpose

`engine::tools` provides developer utilities, in-game diagnostic interfaces, and profiling visualization tools. It enables developers and testers to inspect internal engine health, framerate stability, and network telemetry in real-time directly within the game viewport.

Key responsibilities include:
- **Metrics Overlay (`MetricsOverlay`):** An on-screen HUD panel displaying real-time FPS, frame busy time (work budget vs. sleep), network round-trip ping, jitter, packet loss percentage, and upstream/downstream bandwidth.
- **Event-Driven Telemetry:** Subscribes to `engine::core::EventBus` for `MetricsUpdated` snapshots dispatched every second by `engine::core::Metrics`.
- **In-Game Toggle Control:** Interacts with `engine::input::InputMap` to toggle visibility dynamically (mapped by default to key `F3`).
- **Detailed Reference:** See [Metrics Architecture and Network Telemetry](metrics.md) for in-depth explanations of measured variables and server log outputs.

---

## Utility & Audience

### Architectural Benefits
- **Zero Simulation Interference:** The overlay operates purely as a passive observer. It never polls sockets or inspects simulation internals directly; it listens exclusively to decoupled telemetry events on the `EventBus`.
- **Immediate Visual Diagnostics:** Eliminates the need to attach heavy external debuggers or profilers during gameplay tests, making network regressions or frame spikes immediately visible to QA and developers.
- **Translucent Presentation:** Uses `IGraphicsBackend` primitive drawing (`drawRect`, `drawText`) to render over the running game without interfering with active gameplay layers.

### Target Audience
- **Game Developers & Designers:** For performance profiling, verifying 60 FPS budgets, and monitoring tick stability.
- **Network Engineers & QA:** For diagnosing simulated packet drops, high latency conditions, and bandwidth usage during multiplayer tests.

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (requires `EventBus`, `Subscription`, `Metrics`, `MetricsSnapshot`).
  - `engine::render` (requires `IGraphicsBackend`, `FontId`, `Color`, `RectDraw`, `TextDraw`).
  - `engine::input` (requires action mapping for toggling visibility).
- **External Dependencies:**
  - SFML 3 (transitively through `engine::render` and `engine::input`).

---

## Usage Guide

### 1. Initializing and Wiring the Metrics Overlay

Create the `EventBus`, `Metrics` collector, and `MetricsOverlay` instance:

```cpp
#include "engine/core/EventBus.hpp"
#include "engine/core/metrics/Metrics.hpp"
#include "engine/tools/MetricsOverlay.hpp"
#include "engine/render/SfmlRenderer.hpp"
#include "engine/input/InputMap.hpp"
#include "engine/input/SfmlInput.hpp"

void setupMetricsTool(engine::render::SfmlRenderer& renderer, engine::core::EventBus& bus) {
    // 1. Initialize telemetry collector and overlay tool
    engine::core::Metrics metrics(bus);
    
    // Load a monospace font for crisp tabular alignment
    engine::render::FontId font = renderer.loadFontFromFile("assets/fonts/LiberationMono-Regular.ttf");
    engine::tools::MetricsOverlay overlay(bus, font);

    // 2. Configure F3 key toggle in InputMap
    engine::input::InputMap inputMap;
    inputMap.bindKey("metrics", "F3");
    engine::input::SfmlInput input(inputMap);

    // In render loop:
    while (renderer.isOpen()) {
        renderer.handleEvent();
        input.update(renderer.hasFocus());

        // Toggle overlay visibility on press
        if (input.justPressed("metrics")) {
            overlay.toggle();
        }

        renderer.beginFrame();
        // ... draw game world ...

        // Draw metrics overlay on top if visible
        if (overlay.visible()) {
            overlay.draw(renderer);
        }

        renderer.endFrame();
    }
}
```

For more details on interpreting metrics values and simulating loss/latency, see [Metrics Guide](metrics.md).
