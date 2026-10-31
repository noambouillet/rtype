# Input Module (`engine::input`)

## Overview / Purpose

`engine::input` provides device abstraction and action-mapping facilities for keyboard and gamepad inputs. It converts raw hardware peripheral events into game-level semantic actions (such as `"move_up"`, `"fire"`, or `"toggle_metrics"`).

Key responsibilities include:
- **Action Mapping (`InputMap`):** Associates high-level action names with specific keyboard keys, controller buttons, or analog joystick axes.
- **Input Backend Abstraction (`IInputBackend`):** Interface for querying action states (`isDown()` for continuous presses and `justPressed()` for single-frame trigger events).
- **SFML Input Implementation (`SfmlInput`):** Concrete input backend reading keyboard and joystick state via SFML.
- **Deadzone Calibration:** Configurable deadzone handling for analog joystick axes to counteract stick drift and noisy hardware.
- **Window Focus Integration:** Protects against background input pollution by verifying window focus before processing keystrokes.
- **Persistence & Serialization:** Serializes and deserializes keybindings to and from string maps, making it straightforward to persist user bindings in configuration files.

---

## Utility & Audience

### Architectural Benefits
- **Strict Decoupling from Render Subsystem:** `engine::input` does **not** depend on `engine::render`. The game loop queries window focus from `SfmlRenderer` and passes a simple boolean into `SfmlInput::update(hasFocus)`. This enables clean architectural layering and makes it possible to feed simulated input scripts without initializing a window.
- **Accessibility & Customization (GAG / XAG):** Players can rebind any keyboard or gamepad button. Deadzone configuration protects against hardware drift, ensuring smooth control.
- **Semantic Independence:** Gameplay systems never inspect `sf::Keyboard::Key` or raw scan codes; they query logical actions like `input.isDown("fire")`.

### Target Audience
- **Graphical Client Applications (`r-type_client`):** Translates physical player input into entity control requests and UI navigation commands.
- **Engine Debug Tools (`engine::tools`):** Listens for diagnostic hotkeys (e.g., F3 for the metrics overlay).

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (core types and configuration support).
- **External Dependencies:**
  - **SFML 3:** Linked privately (`SFML::Window`, `SFML::System`).

---

## Usage Guide

### 1. Configuring Action Bindings with `InputMap`

Define action names and bind them to keyboard keys and gamepad buttons/axes:

```cpp
#include "engine/input/InputMap.hpp"

engine::input::InputMap configureBindings() {
    engine::input::InputMap map;

    // Bind keyboard keys
    map.bindKey("move_up", "Z");
    map.bindKey("move_down", "S");
    map.bindKey("move_left", "Q");
    map.bindKey("move_right", "D");
    map.bindKey("fire", "Space");
    map.bindKey("metrics", "F3");

    // Bind gamepad buttons and analog sticks (Joystick 0)
    map.bindButton("fire", 0); // Button 'A' / Cross
    map.bindAxis("move_up", engine::input::JoystickAxis::Y, engine::input::AxisDirection::Negative);
    map.bindAxis("move_down", engine::input::JoystickAxis::Y, engine::input::AxisDirection::Positive);
    map.bindAxis("move_left", engine::input::JoystickAxis::X, engine::input::AxisDirection::Negative);
    map.bindAxis("move_right", engine::input::JoystickAxis::X, engine::input::AxisDirection::Positive);

    return map;
}
```

### 2. Loading and Saving Bindings via Configuration Maps

Integrate with file configuration parsers:

```cpp
#include <map>
#include <string>
#include "engine/input/InputMap.hpp"

void loadCustomBindings(engine::input::InputMap& map, const std::map<std::string, std::string>& config) {
    // Overwrites bindings from config key-value pairs (e.g., key_fire = "Space")
    map.loadFromMap(config);
}

void persistBindings(const engine::input::InputMap& map, std::map<std::string, std::string>& config) {
    map.saveToMap(config);
}
```

### 3. Polling Input States in the Game Loop

Instantiate `SfmlInput`, update its state each frame with window focus, and query actions:

```cpp
#include <iostream>
#include "engine/input/SfmlInput.hpp"
#include "engine/render/SfmlRenderer.hpp"

void gameLoop(engine::render::SfmlRenderer& window, engine::input::InputMap map) {
    engine::input::SfmlInput input(std::move(map), 0); // Joystick ID 0

    while (window.isOpen()) {
        window.handleEvent();

        // Update input state passing window focus state
        input.update(window.hasFocus());

        if (input.justPressed("metrics")) {
            std::cout << "F3 pressed: Toggling debug metrics panel" << std::endl;
        }

        if (input.isDown("fire")) {
            std::cout << "Weapon trigger held down" << std::endl;
        }

        if (input.isDown("move_up")) {
            // Move player upward
        }
    }
}
```
