# Render Module (`engine::render`)

## Overview / Purpose

`engine::render` provides a 2D graphics rendering subsystem and hardware-accelerated windowing abstractions. It manages visual presentation, texture/font handles, frame lifecycle synchronization, and sprite batching.

Key responsibilities include:
- **Graphics Backend Abstraction (`IGraphicsBackend`):** An abstract interface declaring primitives for texture/font loading, sprite blitting, rectangle rendering, text drawing, and parallax background scrolling.
- **SFML 3 Implementation (`SfmlRenderer`):** Concrete graphics backend implementation utilizing SFML 3 with the Pimpl pattern (`Implementation`), completely preventing SFML headers from leaking into engine interfaces or gameplay client code.
- **Window Management & Event Polling:** Window creation, fullscreen/resolution sizing, title management, framerate capping, and OS window event handling (`handleEvent()`).
- **Render ECS Components:** Defines `Sprite` (texture ID, texture sub-rect, origin, scaling, rotation, sorting layers) and `Animation` (sprite sequences, frame times, looping state).
- **Render System (`RenderSystem`):** An ECS system that queries entities containing `Position` and `Sprite`/`Animation`, sorts sprites by layer, updates animation frames by delta time, and submits draw commands to the graphics backend.

---

## Utility & Audience

### Architectural Benefits
- **Pimpl Header Isolation:** By shielding SFML headers inside `SfmlRenderer.cpp`, game code and other engine modules do not include third-party graphics headers. This drastically improves compilation times and eliminates symbol conflicts.
- **Pluggable Render Backends:** Game simulation and view systems interact solely with `IGraphicsBackend`, allowing the underlying rendering engine (e.g., SFML, SDL, Vulkan, DirectX) to be replaced or mocked without modifying game code.
- **Modular Build Separation:** Headless applications (such as `r-type_server`) disable `ENGINE_BUILD_RENDER` completely, producing a minimal executable free of graphics driver and windowing dependencies.

### Target Audience
- **Graphical Client Applications (`r-type_client`):** Used to initialize the game window, render gameplay entities, display backgrounds, and draw HUD elements.
- **Engine Modules:**
  - `engine::assets`: Relies on `IGraphicsBackend` to load textures and fonts during manifest loading.
  - `engine::tools`: Relies on `IGraphicsBackend` to render debug panels and metrics overlays.

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (requires ECS `Coordinator`, `System`, and `Position` component).
- **External Dependencies:**
  - **SFML 3:** Linked privately (`SFML::Graphics`, `SFML::Window`, `SFML::System`).

---

## Usage Guide

### 1. Window Initialization & Frame Loop

Initialize the `SfmlRenderer` and drive the main frame lifecycle:

```cpp
#include "engine/render/SfmlRenderer.hpp"

int main() {
    engine::render::WindowConfig config{
        .width = 1280,
        .height = 720,
        .title = "R-Type Client",
        .frameRateLimit = 60
    };

    engine::render::SfmlRenderer renderer(config);

    while (renderer.isOpen()) {
        renderer.handleEvent();

        renderer.beginFrame();

        // Draw a solid rectangle or visual elements
        renderer.drawRect({
            .x = 100.0f,
            .y = 100.0f,
            .width = 200.0f,
            .height = 50.0f,
            .color = {255, 0, 0, 255}
        });

        renderer.endFrame();
    }

    return 0;
}
```

### 2. Loading Textures and Drawing Sprites

Load an asset into a texture handle and issue draw calls:

```cpp
#include "engine/render/GraphicsBackend.hpp"
#include "engine/render/SfmlRenderer.hpp"

void drawPlayer(engine::render::IGraphicsBackend& backend) {
    // Load texture file to obtain handle
    engine::render::TextureId shipTex = backend.loadTextureFromFile("assets/ship.png");

    engine::render::SpriteDraw sprite{
        .textureId = shipTex,
        .region = {.position = {0, 0}, .size = {64, 32}},
        .x = 250.0f,
        .y = 180.0f,
        .originX = 32.0f,
        .originY = 16.0f,
        .scale = 2.0f,
        .rotation = 0.0f
    };

    backend.drawSprite(sprite);
}
```

### 3. Integrating with the ECS `RenderSystem`

Register `RenderSystem` with the `Coordinator` to automate sprite rendering:

```cpp
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/render/Components.hpp"
#include "engine/render/RenderSystem.hpp"

void setupRendering(engine::core::Coordinator& coordinator, engine::render::SfmlRenderer& renderer) {
    coordinator.registerComponent<engine::core::Position>();
    coordinator.registerComponent<engine::render::Sprite>();
    coordinator.registerComponent<engine::render::Animation>();

    auto& renderSystem = coordinator.registerSystem<engine::render::RenderSystem>();
    coordinator.setSystemSignature<engine::render::RenderSystem>(
        engine::render::RenderSystem::signature(coordinator)
    );

    // Create an animated sprite entity
    engine::core::Entity entity = coordinator.createEntity();
    coordinator.addComponent(entity, engine::core::Position{100.0f, 300.0f});
    coordinator.addComponent(entity, engine::render::Sprite{
        .texture = renderer.loadTextureFromFile("assets/r-typesheet1.gif"),
        .region = {.position = {0, 0}, .size = {33, 17}},
        .scale = 2.0f,
        .layer = 1
    });

    // In your game loop:
    float deltaTime = 1.0f / 60.0f;
    renderer.beginFrame();
    renderSystem.render(coordinator, renderer, deltaTime);
    renderer.endFrame();
}
```
