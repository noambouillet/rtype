# Engine Logging Architecture

## Overview

The engine utilizes a **Service Locator** pattern for observability and diagnostics through `engine::core::Logger`. Rather than relying on a rigid Singleton, the architecture uses a shared instance (`std::shared_ptr<engine::core::Logger>`) injected at application startup via `engine::core::Logger::setDefaultLogger()`.

### Key Benefits

1. **Decoupled Lifecycle & Inversion of Control**: The application entry point (`main`) decides where logs go (console output, specific log file, or test stream) and controls the verbosity level before registering the instance.
2. **Safe Fallback**: If `engine::core::Logger::defaultLogger()` is called before explicit configuration (or during unit tests), it safely returns a fallback logger outputting to `std::cout` at `INFO` level, avoiding null pointer dereferences and Static Initialization Order Fiasco (SIOF).
3. **Testability & Mocking**: Unit and integration tests can instantiate distinct `Logger` instances or substitute memory/string streams without mutating global state permanently.

---

## The 4 Strict Log Levels

The logging subsystem enforces four distinct log levels. The default minimum log level is **`INFO`**.

```
DEBUG < INFO < WARN < ERROR
```

| Level | Severity | Visibility by Default | Semantic Purpose | Typical Examples |
|---|---|---|---|---|
| **`DEBUG`** | Low | Disabled (Enabled with `--debug`) | Granular diagnostic details, packet telemetry, resource preloading, ECS type registration. | `"Loaded texture asset 'player' from assets/ship.png"`, `"ECS: Registered component type 'Position'"`, `"GameClient: Received packet of 48 bytes"` |
| **`INFO`** | Normal | **Enabled** | High-level application lifecycle events only. | `"GameServer ready and listening on UDP port 6767"`, `"Starting R-Type client application..."`, `"Client disconnected gracefully"` |
| **`WARN`** | High | **Enabled** | Recoverable issues, abnormal runtime events, missing optional resources, fallbacks triggered. | `"AssetManager: Missing texture asset 'alien', using fallback ID"`, `"UdpServer: Packet dropped, pending buffer full"`, `"Session for player 2 timed out after inactivity"` |
| **`ERROR`** | Critical | **Enabled** | Severe failures preventing an operation from completing; frequently precedes an exception or abort. | `"AssetManager: Unable to open manifest: assets/manifest.json"`, `"Coordinator: Max entities reached"`, `"ServerNetworkService: getrandom syscall failed"` |

---

## Anti-Spam & DRY Logging Guidelines

1. **Log Once at the Source**:
   - Do **not** repeat logs up the call stack.
   - If a lower-level function (e.g., `parseJsonFile` in `AssetManager`) detects a failure and logs an `ERROR`, the calling function (`loadManifest`) must **not** log the failure again.
2. **Strict `INFO` Discipline**:
   - `INFO` must never be used for per-frame, per-entity, or per-asset operations.
   - Resource loading messages ("Loaded texture X", "Preloaded group Y") and ECS registrations belong in `DEBUG`.
3. **No Direct Standard Streams**:
   - Standard output and error streams (`std::cout`, `std::cerr`) are strictly reserved for the `Logger` implementation itself and initial command-line usage/help text. All engine and gameplay code must use the logging service.

---

## Code Examples

### 1. Initializing the Logger in an Executable (`main.cpp`)

The executable parses command-line flags (`--debug`, `--log-file <path>`), creates the logger, and registers it as the default:

```cpp
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "engine/core/Logger.hpp"

int main(int argc, char** argv)
{
    // 1. Parse command-line options
    bool isDebugMode = false;
    std::string logFilePath = "";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            isDebugMode = true;
        } else if (arg == "--log-file" && i + 1 < argc) {
            logFilePath = argv[++i];
        } else if (arg.starts_with("--log-file=")) {
            logFilePath = arg.substr(11);
        }
    }

    // 2. Instantiate the logger (file stream or console output)
    std::shared_ptr<engine::core::Logger> logger;
    if (logFilePath.empty()) {
        logger = std::make_shared<engine::core::Logger>(std::cout);
    } else {
        logger = std::make_shared<engine::core::Logger>(std::filesystem::path(logFilePath));
    }

    // 3. Set verbose level if --debug was passed (switches min level from INFO to DEBUG)
    if (isDebugMode) {
        logger->setVerbose(true);
    }

    // 4. Inject as the default logger (Service Locator)
    engine::core::Logger::setDefaultLogger(logger);

    logger->info("Application initialized successfully");
    // ... run application ...
    return 0;
}
```

### 2. Using the Logger Anywhere in Engine or Game Code

Subsystems, ECS systems, and utility classes access the active logger using the static locator:

```cpp
#include "engine/core/Logger.hpp"

void loadGameLevel(const std::string& levelName)
{
    // Granular step -> DEBUG
    engine::core::Logger::defaultLogger().debug("Loading assets for level: " + levelName);

    if (!loadAssets(levelName)) {
        // Recoverable warning -> WARN
        engine::core::Logger::defaultLogger().warn("Optional assets missing for level: " + levelName);
    }

    // Major lifecycle milestone -> INFO
    engine::core::Logger::defaultLogger().info("Level '" + levelName + "' loaded and ready");
}

void processNetworkPacket(const std::vector<uint8_t>& packet)
{
    if (packet.empty()) {
        // Error logged directly at source -> ERROR
        engine::core::Logger::defaultLogger().error("Received malformed empty packet");
        return;
    }

    // High frequency / telemetry -> DEBUG
    engine::core::Logger::defaultLogger().debug("Processed datagram of size " + std::to_string(packet.size()));
}
```
