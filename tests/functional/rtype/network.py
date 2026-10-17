"""
DegradedLink: a UDP proxy placed between test clients and the server that
simulates a bad network (latency, jitter, loss, duplication, reordering and a
bandwidth cap), in both directions.

It plays the role of netem/clumsy but needs no root access, works on every OS
and is seeded, so a failing run can be replayed. See tests/functional/netem.sh
for the same conditions applied to the real loopback interface.
"""

import heapq
import random
import select
import socket
import threading
import time
from dataclasses import dataclass, field

from .harness import LOCALHOST


@dataclass
class Conditions:
    latency: float = 0.0      # one-way delay in seconds
    jitter: float = 0.0       # +/- random delay in seconds, which also reorders packets
    loss: float = 0.0         # probability to drop a datagram
    duplicate: float = 0.0    # probability to send a datagram twice
    bandwidth: int = 0        # bytes per second in each direction, 0 = unlimited
    queue_delay: float = 1.0  # a datagram waiting longer than this for bandwidth is dropped


@dataclass
class DirectionStats:
    sent: int = 0
    dropped_loss: int = 0
    dropped_queue: int = 0
    duplicated: int = 0
    bytes: int = 0

    def __str__(self):
        return (f"{self.sent} sent, {self.dropped_loss} lost, {self.dropped_queue} dropped by the "
                f"bandwidth queue, {self.duplicated} duplicated, {self.bytes} bytes")


@dataclass
class LinkStats:
    upstream: DirectionStats = field(default_factory=DirectionStats)    # client -> server
    downstream: DirectionStats = field(default_factory=DirectionStats)  # server -> client


class DegradedLink:
    def __init__(self, server_address, conditions, seed=4242):
        self.server_address = server_address
        self.conditions = conditions
        self.stats = LinkStats()
        self._random = random.Random(seed)
        self._front = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._front.bind((LOCALHOST, 0))
        self.address = self._front.getsockname()
        self._upstream_by_client = {}
        self._client_by_upstream = {}
        self._queue = []
        self._order = 0
        self._link_free_at = {"up": 0.0, "down": 0.0}
        self._running = True
        self._thread = threading.Thread(target=self._run, daemon=True)

    def __enter__(self):
        self._thread.start()
        return self

    def __exit__(self, *exc):
        self._running = False
        self._thread.join()
        self._front.close()
        for sock in self._upstream_by_client.values():
            sock.close()

    def _schedule(self, direction, sock, data, destination):
        conditions = self.conditions
        stats = self.stats.upstream if direction == "up" else self.stats.downstream
        if self._random.random() < conditions.loss:
            stats.dropped_loss += 1
            return
        copies = 2 if self._random.random() < conditions.duplicate else 1
        stats.duplicated += copies - 1
        now = time.monotonic()
        for _ in range(copies):
            departure = now
            if conditions.bandwidth:
                departure = max(now, self._link_free_at[direction])
                if departure - now > conditions.queue_delay:
                    stats.dropped_queue += 1
                    continue
                self._link_free_at[direction] = departure + len(data) / conditions.bandwidth
            delay = conditions.latency + self._random.uniform(-conditions.jitter, conditions.jitter)
            self._order += 1
            heapq.heappush(self._queue, (departure + max(delay, 0.0), self._order, sock, data, destination, stats))

    def _upstream_for(self, client):
        if client not in self._upstream_by_client:
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.bind((LOCALHOST, 0))
            self._upstream_by_client[client] = sock
            self._client_by_upstream[sock] = client
        return self._upstream_by_client[client]

    def _deliver_due(self):
        now = time.monotonic()
        while self._queue and self._queue[0][0] <= now:
            _, _, sock, data, destination, stats = heapq.heappop(self._queue)
            try:
                sock.sendto(data, destination)
                stats.sent += 1
                stats.bytes += len(data)
            except OSError:
                pass

    def _run(self):
        while self._running:
            timeout = 0.01
            if self._queue:
                timeout = min(timeout, max(self._queue[0][0] - time.monotonic(), 0.0))
            readable, _, _ = select.select([self._front, *self._client_by_upstream], [], [], timeout)
            for sock in readable:
                try:
                    data, sender = sock.recvfrom(65535)
                except OSError:
                    continue
                if sock is self._front:
                    self._schedule("up", self._upstream_for(sender), data, self.server_address)
                else:
                    self._schedule("down", self._front, data, self._client_by_upstream[sock])
            self._deliver_due()
