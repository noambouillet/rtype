# Audio Module (`engine::audio`)

## Overview / Purpose

`engine::audio` delivers a cross-platform sound and music playback subsystem. It abstracts hardware audio output, streaming music tracks, polyphonic sound effects, and spatial audio triggers.

Key responsibilities include:
- **Audio Backend Abstraction (`IAudioBackend`):** Defines high-level contracts for background music playback (`playMusic()`, `stopMusic()`) and sound effect triggering (`playSound()`).
- **SFML 3 Implementation (`SfmlAudio`):** Implements `IAudioBackend` backed by SFML 3's audio engine using the Pimpl idiom (`Implementation`) to isolate platform audio drivers and SFML dependencies.
- **Audio ECS Components (`Sound`):** Provides a declarative component containing the sound identifier and volume level.
- **Audio System (`AudioSystem`):** An ECS system that queries entities with `Sound` components, issues sound playback requests to the audio backend, and manages continuous background music streams.

---

## Utility & Audience

### Architectural Benefits
- **Zero Audio Leaks in Logic:** Gameplay code triggers audio cues purely through events, direct semantic backend calls, or ECS components without concerning itself with audio sample formats, buffer streaming, or sound device lifetimes.
- **Headless Isolation:** Disabled on headless servers (`ENGINE_BUILD_AUDIO=OFF`), ensuring server binaries do not require audio drivers (ALSA, PulseAudio, DirectSound) to run in containerized or cloud environments.
- **Decoupled Asset Pathing:** Operates relative to a configurable asset root path, simplifying audio path resolution across different operating systems.

### Target Audience
- **Graphical Client Applications (`r-type_client`):** Used to play background music themes, player fire sounds, explosion sound effects, and UI feedback clicks.
- **Audio Feedback Systems:** Subsystems that translate gameplay events (such as collisions or weapon discharges) into audio feedback.

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (requires `Coordinator`, `System`, and core types).
- **External Dependencies:**
  - **SFML 3:** Linked privately (`SFML::Audio`, `SFML::System`).

---

## Usage Guide

### 1. Initializing the Audio Backend

Instantiate `SfmlAudio` with the asset root directory:

```cpp
#include <filesystem>
#include "engine/audio/SfmlAudio.hpp"

void initAudio() {
    // Assets are located in ./assets/
    engine::audio::SfmlAudio audio(std::filesystem::path("assets"));

    // Play a sound effect at 80% volume
    audio.playSound("sounds/laser.wav", 80.0f);

    // Play background looping music at 50% volume
    audio.playMusic("sounds/stage1.ogg", 50.0f);

    // Stop background music when leaving the stage
    audio.stopMusic();
}
```

### 2. Using the ECS `AudioSystem`

Integrate audio playback directly with the Entity Component System:

```cpp
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/audio/Components.hpp"
#include "engine/audio/AudioSystem.hpp"
#include "engine/audio/SfmlAudio.hpp"

void setupAudioSystem(engine::core::Coordinator& coordinator, engine::audio::IAudioBackend& backend) {
    coordinator.registerComponent<engine::audio::Sound>();

    auto& audioSystem = coordinator.registerSystem<engine::audio::AudioSystem>();
    coordinator.setSystemSignature<engine::audio::AudioSystem>(
        engine::audio::AudioSystem::signature(coordinator)
    );

    // Set stage background music
    audioSystem.setMusic("sounds/stage1.ogg", 60.0f);

    // Create an entity that produces a sound upon creation
    engine::core::Entity explosion = coordinator.createEntity();
    coordinator.addComponent(explosion, engine::audio::Sound{
        .name = "sounds/explosion.wav",
        .volume = 100.0f
    });

    // In the update loop:
    audioSystem.play(coordinator, backend);
}
```
