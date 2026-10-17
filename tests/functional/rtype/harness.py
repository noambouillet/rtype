"""
Tools shared by every functional test:

- ServerProcess starts the real r-type_server binary on a free port.
- TestClient is a scripted UDP client that speaks the protocol like the real one.
- ServerTestCase is the unittest base class that wires both together.
"""

import os
import random
import signal
import socket
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path

from . import protocol as proto

REPO_ROOT = Path(__file__).resolve().parents[3]
LOCALHOST = "127.0.0.1"


def build_dir():
    """Directory holding the binaries: $RTYPE_BUILD_DIR, or <repo>/build by default."""
    return Path(os.environ.get("RTYPE_BUILD_DIR", REPO_ROOT / "build")).resolve()


def binary(name):
    suffix = ".exe" if sys.platform == "win32" else ""
    path = build_dir() / (name + suffix)
    if not path.is_file():
        raise unittest.SkipTest(f"{path} not found: build the project or set RTYPE_BUILD_DIR")
    return path


def free_udp_port():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as probe:
        probe.bind((LOCALHOST, 0))
        return probe.getsockname()[1]


class ServerProcess:
    """The real server, running in the background with its log in a temporary file."""

    def __init__(self, port=None):
        self.port = port or free_udp_port()
        self.address = (LOCALHOST, self.port)
        self._logdir = tempfile.TemporaryDirectory(prefix="rtype-functional-")
        self.log_path = Path(self._logdir.name) / "server.log"
        self.process = None

    def start(self, timeout=5.0):
        with open(self.log_path, "wb") as log:
            self.process = subprocess.Popen([str(binary("r-type_server")), str(self.port)],
                                            stdout=log, stderr=subprocess.STDOUT)
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if "listening" in self.log():
                return self
            if self.process.poll() is not None:
                raise RuntimeError(f"server exited with code {self.process.returncode}:\n{self.log()}")
            time.sleep(0.02)
        self.stop()
        raise RuntimeError(f"server did not start within {timeout}s:\n{self.log()}")

    def is_running(self):
        return self.process is not None and self.process.poll() is None

    def log(self):
        try:
            return self.log_path.read_text(errors="replace")
        except FileNotFoundError:
            return ""

    def stop(self, timeout=3.0):
        """Asks the server to stop with SIGTERM and returns its exit code."""
        if self.process is None:
            return None
        if self.process.poll() is None:
            self.process.send_signal(signal.SIGTERM if hasattr(signal, "SIGTERM") else signal.SIGINT)
            try:
                self.process.wait(timeout)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        return self.process.returncode

    def cleanup(self):
        self.stop()
        self._logdir.cleanup()


class TestClient:
    """A scripted client. It sends to `target`, which is the server or a DegradedLink."""

    INPUT_INTERVAL = 0.05
    JOIN_RETRY_INTERVAL = 0.5
    JOIN_ATTEMPTS = 10

    def __init__(self, target):
        self.target = target
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((LOCALHOST, 0))
        self.nonce = random.getrandbits(64)
        self.session_id = None
        self.player_id = None
        self.sequence = 0
        self.received = []
        self.bytes_received = 0

    def close(self):
        self.sock.close()

    def send(self, datagram):
        self.sock.sendto(datagram, self.target)

    def next_sequence(self):
        self.sequence = (self.sequence + 1) & 0xFFFFFFFF or 1
        return self.sequence

    def recv(self, timeout):
        """Returns the next parsed packet, or None after `timeout` seconds."""
        self.sock.settimeout(max(timeout, 0.001))
        try:
            data, _ = self.sock.recvfrom(65535)
        except (socket.timeout, ConnectionResetError):
            return None
        self.bytes_received += len(data)
        packet = proto.parse(data)
        if packet is None:
            raise AssertionError(f"server sent a datagram that breaks the RFC header rules: {data.hex()}")
        self.received.append(packet)
        return packet

    def wait_for(self, packet_type, timeout=2.0, predicate=None):
        """Waits for a packet of `packet_type` (and matching `predicate`), keeping the others."""
        deadline = time.monotonic() + timeout
        while (remaining := deadline - time.monotonic()) > 0:
            packet = self.recv(remaining)
            if packet and packet.type == packet_type and (predicate is None or predicate(packet)):
                return packet
        return None

    def drain(self, duration):
        """Collects everything the server sends during `duration` seconds."""
        packets = []
        deadline = time.monotonic() + duration
        while (remaining := deadline - time.monotonic()) > 0:
            packet = self.recv(remaining)
            if packet:
                packets.append(packet)
        return packets

    def join(self, nonce=None):
        """Joins like the real client: same nonce every 500 ms, 10 attempts. Returns the reply."""
        if nonce is not None:
            self.nonce = nonce
        same_nonce = lambda packet: proto.parse_join_reply(packet)[0] == self.nonce
        for _ in range(self.JOIN_ATTEMPTS):
            self.send(proto.join_request(self.nonce))
            deadline = time.monotonic() + self.JOIN_RETRY_INTERVAL
            while (remaining := deadline - time.monotonic()) > 0:
                packet = self.recv(remaining)
                if packet and packet.type in (proto.JOIN_ACCEPTED, proto.JOIN_REJECTED) and same_nonce(packet):
                    if packet.type == proto.JOIN_ACCEPTED:
                        self.session_id = packet.session_id
                        self.player_id = proto.parse_join_reply(packet)[1]
                    return packet
        return None

    def send_input(self, buttons):
        self.send(proto.player_input(self.session_id, self.next_sequence(), buttons))

    def play(self, duration, buttons=0):
        """Holds `buttons` for `duration` seconds, refreshing input every 50 ms like the real client."""
        packets = []
        deadline = time.monotonic() + duration
        next_input = time.monotonic()
        while (now := time.monotonic()) < deadline:
            if now >= next_input:
                self.send_input(buttons)
                next_input = now + self.INPUT_INTERVAL
            packet = self.recv(min(next_input, deadline) - now)
            if packet:
                packets.append(packet)
        return packets

    def leave(self):
        self.send(proto.disconnect(self.session_id, self.next_sequence()))


def snapshots(packets):
    return [proto.parse_snapshot(p) for p in packets if p.type == proto.WORLD_SNAPSHOT]


class ServerTestCase(unittest.TestCase):
    """Starts a fresh server for every test and checks it is still alive afterwards."""

    def setUp(self):
        self.server = ServerProcess().start()
        self.addCleanup(self.server.cleanup)

    def tearDown(self):
        if not self.server.is_running():
            self.fail(f"server died (exit code {self.server.process.returncode}):\n{self.server.log()}")

    def client(self, target=None):
        client = TestClient(target or self.server.address)
        self.addCleanup(client.close)
        return client

    def joined_client(self, target=None):
        client = self.client(target)
        reply = client.join()
        self.assertIsNotNone(reply, "no answer to JOIN_REQUEST")
        self.assertEqual(reply.type, proto.JOIN_ACCEPTED)
        return client
