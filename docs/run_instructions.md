# Run Instructions

This guide explains how to start, configure, and operate both the **R-Type Server** and **R-Type Client** binaries.

---

## 1. Server Execution

The `r-type_server` is a headless, authoritative UDP game server. It manages client sessions (up to 4 simultaneous players), executes the physics and gameplay simulation at fixed 60 Hz ticks, and broadcasts world snapshots and event notifications.

### Command-Line Syntax

```bash
./build/r-type_server [udp-port] [--log-file=path] [-h]
```

### Options and Arguments

| Option / Argument | Type | Default Value | Description |
|---|---|---|---|
| `[udp-port]` | Positional integer (`1` - `65535`) | `6767` | The local UDP port the server binds to for incoming client traffic. |
| `--log-file=path` | Key-value flag | Standard output (`stdout`) | Writes server logging output to a designated file instead of the terminal. |
| `-h`, `--help` | Flag | `false` | Displays the usage synopsis and exits immediately with code `0`. |

### Execution Examples

- **Default Launch (Port 6767, console output):**
  ```bash
  ./build/r-type_server
  ```
- **Custom Port:**
  ```bash
  ./build/r-type_server 4242
  ```
- **Custom Port with File Logging:**
  ```bash
  ./build/r-type_server 4242 --log-file=server.log
  ```
- **Windows (PowerShell):**
  ```powershell
  .\build\Release\r-type_server.exe 6767
  ```

### Stopping the Server

The server registers signal handlers for `SIGINT` (Ctrl+C) and `SIGTERM`. Upon receiving an interrupt, it completes the current tick, closes open UDP sockets, logs `"Goodbye..."`, and terminates gracefully.

---

## 2. Client Execution

The `r-type_client` is a graphical SFML 3 client. It sends player inputs to the server, receives replicated world snapshots, runs client-side presentation systems (rendering, audio, particle effects, and local firing prediction), and renders at 60 FPS.

### Command-Line Syntax

```bash
./build/r-type_client [host] [port] [--log-file=path] [-h]
```

### Options and Arguments

| Option / Argument | Type | Default Value | Description |
|---|---|---|---|
| `[host]` | Positional string / IP | `"localhost"` | IP address or hostname of the remote game server. |
| `[port]` | Positional integer (`1` - `65535`) | `6767` | Remote UDP destination port of the server. |
| `--log-file=path` | Key-value flag | Disabled | Directs diagnostic logging to a file. |
| `-h`, `--help` | Flag | `false` | Displays usage synopsis and exits with code `0`. |

The client's argument parser flexibly handles port-only or host-and-port specifications:

### Execution Examples

- **Connect to Localhost on Default Port (6767):**
  ```bash
  ./build/r-type_client
  ```
- **Connect to Localhost on a Custom Port (e.g. 4242):**
  ```bash
  ./build/r-type_client 4242
  ```
- **Connect to Remote Host on Default Port:**
  ```bash
  ./build/r-type_client 192.168.1.50
  ```
- **Connect to Remote Host on Custom Port:**
  ```bash
  ./build/r-type_client 192.168.1.50 4242
  ```
- **Run with Logging to File:**
  ```bash
  ./build/r-type_client localhost 6767 --log-file=client_run.log
  ```
- **Windows (PowerShell):**
  ```powershell
  .\build\Release\r-type_client.exe localhost 6767
  ```

---

## 3. Client Configuration (`settings.cfg`)

Upon launch, the client searches for a `settings.cfg` file in the current working directory. If none exists, it generates a default configuration file automatically.

### File Structure and Default Values

```ini
# Keyboard settings
up = Z
down = S
left = Q
right = D
fire = Space
auto_fire = X

# Joystick settings
joystick_fire = 0
joystick_auto_fire = 1
joystick_deadzone = 30

# Visual settings
screen_flash = true
```

### Configuration Sections

1. **Keyboard Bindings:**
   - Actions: `up`, `down`, `left`, `right`, `fire`, `auto_fire`.
   - Supported keys include alphanumeric characters (`A`-`Z`), direction keys (`Up`, `Down`, `Left`, `Right`), and controls (`Space`, `Enter`, `Escape`).
2. **Joystick / Controller Bindings:**
   - `joystick_deadzone`: Axis deadzone threshold (default: `30` in a range of 0 to 100) to eliminate analog stick drift.
   - `joystick_fire`: Hardware button index for manual shot firing (default: `0`, usually button A / Cross).
   - `joystick_auto_fire`: Hardware button index for toggling auto-fire mode (default: `1`, usually button B / Circle).
3. **Visual & Accessibility Settings:**
   - `screen_flash`: Setting this to `false` disables full-screen explosion animations and high-intensity flash textures, safeguarding players with photosensitive epilepsy or visual sensitivities.

---

## 4. Multi-Client Testing Workflow

To test multiplayer gameplay locally on a single workstation:

1. **Launch Server:**
   Open a terminal and start the server:
   ```bash
   ./build/r-type_server 6767
   ```
2. **Launch First Player (Player 0):**
   Open a second terminal and launch the first client:
   ```bash
   ./build/r-type_client localhost 6767
   ```
   The client will send a `JOIN_REQUEST` and be assigned `Player 0` (rendered with Player 1 color scheme).
3. **Launch Additional Players (Up to 4):**
   In subsequent terminals, start additional client instances:
   ```bash
   ./build/r-type_client localhost 6767
   ```
   Each client receives its unique session ID and player index (`Player 1`, `Player 2`, `Player 3`), joining the shared world in real-time.
4. **Disconnection:**
   Closing any client window sends a `DISCONNECT` packet. The server frees that player's slot, dispatches a `PLAYER_LEFT` packet to all remaining participants, and allows new players to join the ongoing match.
