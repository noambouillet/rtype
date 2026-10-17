# Functional tests

The functional tests start the **real** `r-type_server` binary and talk to it
over **real UDP sockets**, exactly like a client would. They check the game from
the outside: if the RFC says "the server answers X", a functional test sends the
packet and waits for X. They do not include or link any C++ code from `src/`.

They also cover the testing requirement of the project annex: the game must keep
working with **2% packet loss or duplication, more than 150 ms of latency and a
5 KB/s link**.

## Why Python, and why a bash script

The tests are written in Python on purpose. They must not reuse the C++ code
they test, Python's standard library already has everything needed (UDP
sockets, binary packing, a test runner, processes, threads), nothing has to be
installed, and the same tests run on Linux and Windows.

There are two ways to degrade the network:

| Tool | Used by | Needs | Affects |
|------|---------|-------|---------|
| `DegradedLink` (Python proxy) | the automated tests, CTest, CI | nothing, any OS | only the test's packets |
| `netem.sh` (bash) | a human playing the real client | Linux, root | everything on 127.0.0.1 |

`netem.sh` is a bash script because it is only a shortcut for the Linux `tc`
command; it is never run by the test suite.

## Quick start

```sh
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
ctest --test-dir build --output-on-failure        # all suites, about 80 s
ctest --test-dir build -j5 --output-on-failure    # suites in parallel, about 40 s
```

The only requirement is Python 3.10 or newer (standard library only, nothing to
install). If CMake does not find Python, it silently skips the tests.

## Running the tests

`ctest` runs one CTest test per file. For anything finer, use the runner
directly:

```sh
python3 tests/functional/run_tests.py                         # everything, binaries in ./build
python3 tests/functional/run_tests.py --build-dir build-tests # binaries somewhere else
python3 tests/functional/run_tests.py test_session            # one file
python3 tests/functional/run_tests.py test_session.LeaveTest  # one class
python3 tests/functional/run_tests.py test_session.LeaveTest.test_free_slot_is_reused
python3 tests/functional/run_tests.py -k latency -k loss      # names containing "latency" or "loss"
python3 tests/functional/run_tests.py -x                      # stop at the first failure
```

| CTest name | File | What it checks |
|------------|------|----------------|
| `functional_cli` | `test_cli.py` | `--help`, invalid arguments (exit code 2), port already in use (exit code 1), clean stop on `SIGTERM` |
| `functional_session` | `test_session.py` | join, nonce echo, first snapshot, 20 snapshots/s, repeated join, 4 players then `SERVER_FULL`, disconnect, slot reuse, 5 s timeout |
| `functional_gameplay` | `test_gameplay.py` | the four directions move the ship, release stops it, input expires after 250 ms, old sequences are ignored, fire rate, projectiles, enemies, every server packet follows the RFC, both players see the same world |
| `functional_robustness` | `test_robustness.py` | every `ERROR` code, error priority, silence on short/foreign datagrams, 3000 random datagrams, a 2000-join flood |
| `functional_degraded_network` | `test_degraded_network.py` | the annex scenarios (see below) and the bandwidth used by the server |

### Reading a failure

A failing assertion says what was expected in game terms, for example:

```text
AssertionError: 52 not greater than or equal to 68.0 : only 52 snapshots out of ~80
  up: 67 sent, 100 lost, 0 dropped by the bandwidth queue, 0 duplicated, 1174 bytes
  down: 145 sent, 214 lost, 0 dropped by the bandwidth queue, 0 duplicated, 26382 bytes
```

If the server crashes during a test, the test fails with its exit code and the
full server log. A test is skipped, not failed, when the binaries are missing:
build first, or pass `--build-dir`.

## Degraded network

`DegradedLink` (`tests/functional/rtype/network.py`) is a small UDP proxy that
sits between the test clients and the server and damages traffic in both
directions:

| `Conditions` field | Effect |
|--------------------|--------|
| `latency` | one-way delay, in seconds |
| `jitter` | random extra delay of ± this value; packets get reordered |
| `loss` | probability of dropping a datagram |
| `duplicate` | probability of sending a datagram twice |
| `bandwidth` | bytes per second per direction (0 = unlimited) |
| `queue_delay` | a datagram waiting longer than this for bandwidth is dropped, like a router queue |

It is seeded, so a failure replays identically. Each scenario connects two
players through the proxy for 4 seconds and checks that nobody is dropped,
snapshots keep arriving, ticks keep increasing and inputs still move the ship.

| Test | Conditions | Snapshots required |
|------|-----------|-------------------|
| `test_2_percent_packet_loss` | 2% loss | 85% |
| `test_2_percent_duplication` | 2% duplication | 90% |
| `test_150ms_latency_with_jitter` | 150 ms ± 30 ms | 75% |
| `test_5kb_per_second_bandwidth` | 5 KB/s | 30% |
| `test_everything_at_once` | all of the above | 25% |
| `test_join_survives_heavy_loss` | 30% loss, join only | — |

`BandwidthTest` prints how much the server sends to one client. Measured on
2026-10-16: about **0.9 KB/s** for an idle player and about **6 KB/s** for a
player who keeps firing, which is above the 5 KB/s of the annex. Under a 5 KB/s
link the game stays playable but loses snapshots. The busy measurement is
reported, not enforced. If you reduce the snapshot size (delta snapshots,
fewer fields, lower rate), turn it into a real assertion:
`self.assertLess(rate, self.BUDGET)`.

### Playing by hand with netem

To feel the conditions with the real client, apply them to the loopback
interface (Linux, needs root, affects everything on `127.0.0.1`):

```sh
sudo tests/functional/netem.sh all     # or: loss, duplicate, latency, bandwidth
./build/r-type_server &
./build/r-type_client
sudo tests/functional/netem.sh off     # do not forget
```

On Windows, [clumsy](https://jagt.github.io/clumsy/) does the same.

## Layout

```text
tests/functional/
├── run_tests.py              entry point used by CTest and by hand
├── netem.sh                  real network degradation for manual testing
├── rtype/
│   ├── protocol.py           packet building and parsing, written from docs/RFC.md
│   ├── harness.py            ServerProcess, TestClient, ServerTestCase
│   └── network.py            DegradedLink and Conditions
├── test_cli.py
├── test_session.py
├── test_gameplay.py
├── test_robustness.py
└── test_degraded_network.py
```

`protocol.py` is written from the RFC on purpose and shares no code with
`src/utils/network/Protocol.cpp`. If both were generated from the same code, a
bug in the C++ encoder would also be in the test and nobody would notice.

## Writing a new test

### 1. Pick the file

Add the test to the file whose topic matches. Create a new `test_<topic>.py`
only for a new topic; files must start with `test_` to be discovered. To make a
new file run under CTest, add its topic to the `foreach` at the end of
`CMakeLists.txt`.

### 2. Write it

Inherit from `ServerTestCase`: each test gets a fresh server on a free port,
and fails automatically if the server crashed.

```python
from rtype import protocol as proto
from rtype.harness import ServerTestCase, snapshots


class ShieldTest(ServerTestCase):
    def test_new_player_starts_with_full_health(self):
        player = self.joined_client()           # joins and asserts JOIN_ACCEPTED
        packets = player.play(0.5)              # 0.5 s of idle input, every 50 ms

        ship = snapshots(packets)[-1].player(player.player_id)
        self.assertIsNotNone(ship)
        self.assertEqual(ship.health, 1)
```

The tools you need:

| Call | Does |
|------|------|
| `self.joined_client(target=None)` | new client that already joined; `target` is the server or a `DegradedLink` |
| `self.client(target=None)` | new client that has not joined yet |
| `client.join(nonce=None)` | joins like the real client (10 attempts, 500 ms apart), returns the reply |
| `client.play(seconds, buttons=0)` | holds buttons and refreshes input every 50 ms; returns received packets |
| `client.send_input(buttons)` | sends one `PLAYER_INPUT` |
| `client.leave()` | sends `DISCONNECT` |
| `client.send(raw_bytes)` | sends any datagram, valid or not |
| `client.wait_for(type, timeout, predicate)` | waits for one packet type |
| `client.drain(seconds)` | collects packets without sending anything |
| `client.received` | every packet received so far |
| `snapshots(packets)` | parsed `WORLD_SNAPSHOT`s from a packet list |
| `proto.build(type, seq, session, payload, version=, magic=, payload_size=)` | crafts any header, including invalid ones |
| `self.server.log()` | the server output so far |

For a network scenario, wrap the clients in a `DegradedLink`:

```python
from rtype.network import Conditions, DegradedLink

with DegradedLink(self.server.address, Conditions(latency=0.2, loss=0.05)) as link:
    player = self.joined_client(link.address)
    packets = player.play(3.0, proto.BUTTON_RIGHT)
print(link.stats.downstream)
```

### 3. Rules that keep the suite reliable

- **Test behaviour, not timing.** Allow a margin: "about 20 snapshots per
  second" is `30 <= count <= 50` over 2 seconds, not `== 40`.
- **Keep sending input** while you wait for something. A session that sends
  nothing for 5 seconds is removed, and buttons are released after 250 ms.
- **Seed every random choice** (`random.Random(1234)`, `DegradedLink(seed=...)`)
  so a failure can be replayed.
- **Write the message for the person who reads the failure:**
  `self.assertTrue(left, "no PLAYER_LEFT after DISCONNECT")`.
- **Check that the test can fail.** Break the expectation once (for example a
  wrong player ID, or 60% loss in a network test) and watch it fail before
  committing.
- **Keep it short.** Most tests take less than a second; only timeouts and
  network scenarios need several seconds.

### 4. When a test finds a bug

Fix the bug in the same branch. If it cannot be fixed now, mark the test with
`@unittest.expectedFailure` and a docstring that explains the bug, and remove
the decorator in the commit that fixes it.
