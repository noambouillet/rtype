# Physics Module (`engine::physics`)

## Overview / Purpose

`engine::physics` provides a deterministic 2D collision detection and spatial bounds subsystem. It evaluates intersections between entities equipped with spatial positions and bounding box colliders.

Key responsibilities include:
- **Collider ECS Component (`Collider`):** Represents Axis-Aligned Bounding Boxes (AABB) with explicit dimensions (`width`, `height`), offset origin, collision layer flags (`layer`), and filter mask flags (`mask`).
- **Collision Detection System (`CollisionSystem`):** An ECS system that queries all entities possessing both `core::Position` and `Collider`, tests layer/mask compatibility, performs AABB geometric intersection tests, and aggregates detected collisions into a list of entity pairs (`CollisionPair`).
- **Bitmask Filtering:** Offers multi-layer collision filtering, enabling game code to selectively allow or prevent interactions (e.g., player lasers collide with enemies, but not with player ships or other player lasers).

---

## Utility & Audience

### Architectural Benefits
- **Deterministic Server Simulation:** The physics engine is lightweight, zero-dependency, and deterministic. It runs seamlessly on authoritative headless servers to guarantee identical collision resolution across diverse client architectures.
- **Decoupled Resolution:** `CollisionSystem` separates collision **detection** from collision **resolution**. The system merely detects overlaps; gameplay systems (such as damage systems, hit systems, or pickup systems) iterate through the resulting `CollisionPair` list to apply game-specific logic (e.g., deducting HP, spawning particle effects, scoring points).
- **Zero Third-Party Overhead:** Eliminates the complexity and memory overhead of large general-purpose physics engines (e.g., Box2D, PhysX) for arcade-style shmup mechanics.

### Target Audience
- **Headless Server Applications (`r-type_server`):** Drives server-side bullet, player, and enemy collision detection.
- **Client Prediction / Local Physics:** Can be used on client builds for local predictive collisions or obstacle interaction.

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (requires `Coordinator`, `System`, and `core::Position`).
- **External Dependencies:**
  - Standard C++20 Library (`<vector>`, `<cstdint>`). No external dependencies.

---

## Usage Guide

### 1. Defining Layers and Configuring Colliders

Define bitmask collision layers and attach `Collider` components to entities:

```cpp
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/physics/Components.hpp"
#include "engine/physics/CollisionSystem.hpp"

// Define layer flags
enum CollisionLayers : uint32_t {
    LAYER_PLAYER  = 1 << 0,
    LAYER_ENEMY   = 1 << 1,
    LAYER_BULLET  = 1 << 2,
};

void setupCollisions(engine::core::Coordinator& coordinator) {
    coordinator.registerComponent<engine::core::Position>();
    coordinator.registerComponent<engine::physics::Collider>();

    auto& collisionSystem = coordinator.registerSystem<engine::physics::CollisionSystem>();
    coordinator.setSystemSignature<engine::physics::CollisionSystem>(
        engine::physics::CollisionSystem::signature(coordinator)
    );

    // Create an enemy entity
    engine::core::Entity enemy = coordinator.createEntity();
    coordinator.addComponent(enemy, engine::core::Position{500.0f, 300.0f});
    coordinator.addComponent(enemy, engine::physics::Collider{
        .width = 32.0f,
        .height = 32.0f,
        .layer = LAYER_ENEMY,
        .mask = LAYER_PLAYER | LAYER_BULLET // Collides with player and bullets
    });

    // Create a player projectile entity
    engine::core::Entity bullet = coordinator.createEntity();
    coordinator.addComponent(bullet, engine::core::Position{510.0f, 305.0f});
    coordinator.addComponent(bullet, engine::physics::Collider{
        .width = 16.0f,
        .height = 8.0f,
        .layer = LAYER_BULLET,
        .mask = LAYER_ENEMY // Collides only with enemies
    });
}
```

### 2. Updating Collisions and Processing Hits

Execute `CollisionSystem::update` during each tick and iterate over detected collision pairs:

```cpp
#include <iostream>
#include "engine/physics/CollisionSystem.hpp"

void handleTickCollisions(engine::core::Coordinator& coordinator, engine::physics::CollisionSystem& collisionSystem) {
    // 1. Detect all overlapping entities
    collisionSystem.update(coordinator);

    // 2. Resolve collisions
    const auto& pairs = collisionSystem.collisions();
    for (const auto& pair : pairs) {
        std::cout << "Collision detected between Entity " 
                  << pair.a << " and Entity " << pair.b << std::endl;
        
        // Apply gameplay damage, destroy projectile, etc.
    }
}
```
