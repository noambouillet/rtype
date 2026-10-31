# Net Module (`engine::net`)

## Overview / Purpose

`engine::net` provides low-latency, asynchronous UDP networking transport for real-time multiplayer communication. It encapsulates operating-system socket APIs and asynchronous I/O loops behind non-blocking, tick-friendly polling interfaces.

Key responsibilities include:
- **Transport Abstractions (`IClientTransport`, `IServerTransport`):** Clear interfaces for sending and polling raw binary datagrams without binding game code to socket primitives.
- **Asynchronous UDP Server (`UdpServer`):** Multi-client UDP listener running an `asio::io_context` background worker thread, managing bounded receive/send queues, endpoint attribution, and packet safety bounds.
- **Asynchronous UDP Client (`UdpClient`):** Lightweight client transport connecting to a designated remote host/port, resolving endpoints, and providing non-blocking datagram polling.
- **Network Data Structures:** Primitive types representing network endpoints (`Endpoint` with address string and 16-bit port) and datagram payloads (`Datagram`).
- **Error Handling & Cross-Platform Support:** Encapsulates socket errors into `NetworkError` and manages platform-specific initialization (such as Windows Sockets 2 `ws2_32` and `_WIN32_WINNT`).

---

## Utility & Audience

### Architectural Benefits
- **Non-Blocking Fixed-Tick Integration:** Both `UdpServer` and `UdpClient` provide `receivePending()`, returning all inbound datagrams accumulated since the last tick. This eliminates callback spaghetti and allows simulation logic to execute deterministically within a 60 Hz game loop.
- **Thread Safety & Queue Protection:** Internal mutexes protect message queues between background I/O worker threads and the main game thread. Limits on max pending packets (`MAX_PENDING = 256`) and sends (`MAX_QUEUED_SENDS = 1024`) protect against buffer bloat and memory exhaustion attacks.
- **Protocol Agnostic:** `engine::net` only moves raw bytes (`std::vector<std::uint8_t>`); it does not enforce any specific serialization format. The game layer (`rtype_protocol`) builds binary packets on top of this transport.

### Target Audience
- **Headless Game Server (`r-type_server`):** Utilizes `UdpServer` to communicate with up to 4 concurrent player sessions.
- **Game Client Network Service (`r-type_client`):** Utilizes `UdpClient` to stream player inputs and receive world state snapshots.

---

## Dependencies

- **Internal Dependencies:**
  - `engine::core` (core logging, error definitions).
- **External Dependencies:**
  - **Asio (standalone):** High-performance asynchronous socket I/O.
  - **Standard Threads (`Threads::Threads`):** Worker thread execution.
  - **Winsock (`ws2_32`):** Required on Windows platforms.

---

## Usage Guide

### 1. Setting Up an Authoritative UDP Server

Initialize `UdpServer` on a specified port and poll messages in the server tick loop:

```cpp
#include <iostream>
#include <vector>
#include "engine/net/UdpServer.hpp"

void runServer() {
    engine::net::UdpServerConfig config{
        .port = 6767,
        .maxDatagramSize = 1024
    };

    engine::net::UdpServer server(config);
    std::cout << "Server listening on UDP port 6767" << std::endl;

    // In fixed tick loop (e.g., 60 Hz):
    while (true) {
        // Poll incoming datagrams
        std::vector<engine::net::Datagram> packets = server.receivePending();
        for (const auto& packet : packets) {
            std::cout << "Received " << packet.bytes.size() 
                      << " bytes from " << packet.sender.address 
                      << ":" << packet.sender.port << std::endl;

            // Echo response back to sender
            server.send(packet.sender, {0x01, 0x02, 0x03});
        }
        
        // ... perform ECS world simulation tick ...
    }
}
```

### 2. Setting Up a UDP Client

Connect `UdpClient` to a remote server host and exchange packets:

```cpp
#include <iostream>
#include <vector>
#include "engine/net/UdpClient.hpp"

void runClient() {
    engine::net::UdpClientConfig config{
        .host = "127.0.0.1",
        .port = 6767,
        .maxDatagramSize = 1024
    };

    engine::net::UdpClient client(config);

    // Send binary request
    std::vector<uint8_t> request = {0xAA, 0xBB, 0xCC};
    client.send(request);

    // Poll incoming responses
    std::vector<std::vector<uint8_t>> responses = client.receivePending();
    for (const auto& payload : responses) {
        std::cout << "Received payload of " << payload.size() << " bytes from server" << std::endl;
    }
}
```
