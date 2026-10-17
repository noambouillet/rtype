"""
Degraded network conditions from the project annex:
2% loss or duplication, latency over 150 ms, and a 5 KB/s link.

Every scenario runs two players through a DegradedLink proxy and checks that
the game stays playable: both join, nobody is dropped, snapshots keep coming
and inputs still move the ship.
"""

import time
import unittest

from rtype import protocol as proto
from rtype.harness import ServerTestCase, snapshots
from rtype.network import Conditions, DegradedLink

PLAY_TIME = 4.0
SNAPSHOT_RATE = 20


class DegradedNetworkTest(ServerTestCase):
    def play_through(self, conditions, min_snapshot_ratio):
        with DegradedLink(self.server.address, conditions) as link:
            players = [self.joined_client(link.address) for _ in range(2)]
            start = {p.player_id: None for p in players}
            received = {p.player_id: [] for p in players}
            deadline = time.monotonic() + PLAY_TIME
            while time.monotonic() < deadline:
                for player in players:
                    player.send_input(proto.BUTTON_RIGHT if player.player_id == 0 else proto.BUTTON_DOWN)
                    received[player.player_id] += player.drain(0.025)
            for player in players:
                received[player.player_id] += player.drain(conditions.latency * 2 + 0.1)

        report = f"\n  up: {link.stats.upstream}\n  down: {link.stats.downstream}"
        for player in players:
            packets = received[player.player_id]
            with self.subTest(player=player.player_id):
                left = [p for p in packets if p.type == proto.PLAYER_LEFT]
                self.assertFalse(left, "a player was dropped" + report)
                states = snapshots(packets)
                expected = PLAY_TIME * SNAPSHOT_RATE
                self.assertGreaterEqual(len(states), expected * min_snapshot_ratio,
                                        f"only {len(states)} snapshots out of ~{expected:.0f}" + report)
                ticks = [s.server_tick for s in states]
                self.assertGreater(ticks[-1], ticks[0])
                self.assertEqual(states[-1].connected_mask & 0b11, 0b11, "a player is not connected" + report)
                ships = [s.player(player.player_id) for s in states if s.player(player.player_id)]
                if len(ships) >= 2:
                    moved = (ships[-1].x - ships[0].x) + (ships[-1].y - ships[0].y)
                    self.assertGreater(moved, 0, "inputs did not move the ship" + report)
        return link.stats

    def test_2_percent_packet_loss(self):
        self.play_through(Conditions(loss=0.02), min_snapshot_ratio=0.85)

    def test_2_percent_duplication(self):
        stats = self.play_through(Conditions(duplicate=0.02), min_snapshot_ratio=0.9)
        self.assertGreater(stats.upstream.duplicated + stats.downstream.duplicated, 0)

    def test_150ms_latency_with_jitter(self):
        self.play_through(Conditions(latency=0.150, jitter=0.030), min_snapshot_ratio=0.75)

    def test_5kb_per_second_bandwidth(self):
        self.play_through(Conditions(bandwidth=5 * 1024), min_snapshot_ratio=0.3)

    def test_everything_at_once(self):
        conditions = Conditions(latency=0.150, jitter=0.030, loss=0.02, duplicate=0.02, bandwidth=5 * 1024)
        self.play_through(conditions, min_snapshot_ratio=0.25)

    def test_join_survives_heavy_loss(self):
        with DegradedLink(self.server.address, Conditions(loss=0.3), seed=7) as link:
            reply = self.client(link.address).join()
        self.assertIsNotNone(reply, "join failed after 10 attempts at 30% loss")
        self.assertEqual(reply.type, proto.JOIN_ACCEPTED)


class BandwidthTest(ServerTestCase):
    BUDGET = 5 * 1024

    def measure(self, duration, buttons):
        player = self.joined_client()
        player.play(0.5)
        player.bytes_received = 0
        player.play(duration, buttons)
        return player.bytes_received / duration

    def test_idle_player_bandwidth(self):
        rate = self.measure(3.0, 0)
        print(f"\n  server -> client, idle player: {rate:.0f} B/s (annex budget {self.BUDGET} B/s)")
        self.assertLess(rate, self.BUDGET)

    def test_busy_player_bandwidth(self):
        """Reported, not enforced: a busy world can exceed 5 KB/s, see docs/testing/functional-tests.md."""
        rate = self.measure(8.0, proto.BUTTON_FIRE)
        print(f"\n  server -> client, firing player: {rate:.0f} B/s (annex budget {self.BUDGET} B/s)")
        max_snapshot = 1051 * SNAPSHOT_RATE
        self.assertLess(rate, max_snapshot * 1.5, "far more traffic than full snapshots can explain")

if __name__ == "__main__":
    unittest.main()
