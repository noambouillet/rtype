# Assets Module (`engine::assets`)

## Overview / Purpose

`engine::assets` delivers a centralized, data-driven asset management system. It loads, parses, caches, and provides access to textures, spritesheets, animations, and sound effects defined in declarative JSON manifests.

Key responsibilities include:
- **Manifest Loading & Parsing (`AssetManager`):** Parses JSON-based asset manifests (`manifest.json`), registering named textures, sprite cuts, animation sequences, and sound effect paths.
- **Group-Based Preloading:** Preloads asset bundles by group (e.g., `"core"`, `"level1"`, `"boss"`) using `IGraphicsBackend` to upload textures to GPU memory upfront and prevent runtime frame drops.
- **Missing Asset Fallbacks:** Integrates an automatic magenta checkerboard fallback texture (`magenta-checkerboard-200px.png`). If an asset file is missing or corrupted, the engine falls back gracefully without crashing the game.
- **Descriptor Retrieval:** Provides fast, cached lookups for `SpriteDescriptor` (texture ID, UV source rect, origin) and `AnimationDescriptor` (frame rects, frame interval, loop behavior).

---

## Utility & Audience

### Architectural Benefits
- **Separation of Content and Code:** Game artists and designers configure textures, sprite dimensions, animation speeds, and audio files entirely in JSON without needing to recompile the C++ project.
- **Resource De-duplication:** Ensures that duplicate requests for the same texture file reference the exact same GPU texture handle (`TextureId`).
- **Resilience Against Missing Assets:** If an asset path is broken during development, the missing asset fallback allows development and QA to proceed uninterrupted.

### Target Audience
- **Graphical Client Applications (`r-type_client`):** Used during game startup to load `assets/manifest.json`, preload level assets, and instantiate animated sprites.
- **Presentation Systems:** View systems (e.g., `ClientWorld`, `EffectSystem`, `SpriteCatalog`) query `AssetManager` to initialize ECS `Sprite` and `Animation` components.

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (core logging, error types).
  - `engine::render` (interacts with `IGraphicsBackend` to allocate texture handles).
- **External Dependencies:**
  - **nlohmann/json:** Fast, modern JSON parser used for manifest deserialization.

---

## Usage Guide

### 1. Structure of `manifest.json`

Assets are declared in a structured JSON manifest file:

```json
{
  "textures": {
    "ship": "r-typesheet1.gif",
    "alien": "r-typesheet5.gif"
  },
  "sprites": {
    "player_ship": {
      "texture": "ship",
      "rect": [0, 0, 33, 17],
      "origin": [16.5, 8.5]
    }
  },
  "animations": {
    "alien_walk": {
      "texture": "alien",
      "frameSeconds": 0.15,
      "loop": true,
      "frames": [
        [0, 0, 33, 33],
        [33, 0, 33, 33],
        [66, 0, 33, 33]
      ]
    }
  },
  "groups": {
    "core": {
      "required": true,
      "textures": ["ship", "alien"],
      "sounds": ["sounds/laser.wav"]
    }
  }
}
```

### 2. Loading the Manifest and Preloading Assets

Initialize `AssetManager` and load the manifest with an active `IGraphicsBackend`:

```cpp
#include "engine/assets/AssetManager.hpp"
#include "engine/render/SfmlRenderer.hpp"

void loadGameAssets(engine::render::SfmlRenderer& renderer) {
    engine::assets::AssetManager assetManager;

    // Parse the manifest and initialize fallback texture
    assetManager.loadManifest("assets/manifest.json", renderer);

    // Preload mandatory core group into GPU memory
    assetManager.preload("core", renderer);
}
```

### 3. Spawning Entities with Cached Asset Descriptors

Query sprite and animation descriptors to populate ECS components:

```cpp
#include "engine/assets/AssetManager.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/render/Components.hpp"

void spawnPlayerShip(engine::core::Coordinator& coordinator, const engine::assets::AssetManager& assets) {
    const auto& desc = assets.getSprite("player_ship");

    engine::core::Entity player = coordinator.createEntity();
    coordinator.addComponent(player, engine::core::Position{100.0f, 200.0f});
    coordinator.addComponent(player, engine::render::Sprite{
        .texture = desc.texture,
        .region = desc.rect,
        .originX = desc.originX,
        .originY = desc.originY,
        .scale = 2.0f,
        .rotation = 0.0f,
        .layer = 2
    });
}
```
