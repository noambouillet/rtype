"""Inputs reach the authoritative world and the world is replicated to the clients."""

import time
import unittest

from rtype import protocol as proto
from rtype.harness import ServerTestCase, snapshots

SUBPIXELS = 16
PLAYER_SPEED = 250  # logical pixels per second, src/server/game/GameConfig.hpp


class MovementTest(ServerTestCase):
    def ship_position(self, client, packets=None):
        received = snapshots(packets if packets is not None else client.play(0.15))
        self.assertTrue(received, "no snapshot received")
        ship = received[-1].player(client.player_id)
        self.assertIsNotNone(ship, "the player's ship is missing from the snapshot")
        return ship.x, ship.y

    def test_each_direction_moves_the_ship(self):
        client = self.joined_client()
        cases = [
            (proto.BUTTON_RIGHT, 1, 0),
            (proto.BUTTON_DOWN, 0, 1),
            (proto.BUTTON_LEFT, -1, 0),
            (proto.BUTTON_UP, 0, -1),
        ]
        for buttons, dx, dy in cases:
            with self.subTest(buttons=buttons):
                start_x, start_y = self.ship_position(client)
                end_x, end_y = self.ship_position(client, client.play(0.3, buttons))
                expected = PLAYER_SPEED * 0.3 * SUBPIXELS
                if dx:
                    self.assertGreater((end_x - start_x) * dx, expected / 2)
                    self.assertEqual(end_y, start_y)
                if dy:
                    self.assertGreater((end_y - start_y) * dy, expected / 2)
                    self.assertEqual(end_x, start_x)

    def test_released_buttons_stop_the_ship(self):
        client = self.joined_client()
        client.play(0.2, proto.BUTTON_RIGHT)
        client.play(0.1)
        first = self.ship_position(client)
        second = self.ship_position(client, client.play(0.3))

        self.assertEqual(first, second)

    def test_input_expires_after_250ms_without_refresh(self):
        client = self.joined_client()
        start_x, _ = self.ship_position(client)
        client.send_input(proto.BUTTON_RIGHT)
        end_x, _ = self.ship_position(client, client.drain(1.0))

        travelled = (end_x - start_x) / SUBPIXELS
        self.assertGreater(travelled, 0, "the single input was not applied")
        self.assertLess(travelled, PLAYER_SPEED * 0.5, "the input was not released after 250 ms")

    def test_older_input_sequences_are_ignored(self):
        client = self.joined_client()
        client.send(proto.player_input(client.session_id, 1000, 0))
        start = self.ship_position(client, client.drain(0.2))
        deadline = time.monotonic() + 0.5
        while time.monotonic() < deadline:
            client.send(proto.player_input(client.session_id, 500, proto.BUTTON_RIGHT))
            client.drain(0.05)

        self.assertEqual(self.ship_position(client, client.drain(0.2)), start)

    def test_ship_stays_inside_the_world(self):
        client = self.joined_client()
        packets = client.play(1.5, proto.BUTTON_UP | proto.BUTTON_LEFT)
        x, y = self.ship_position(client, packets)

        self.assertGreaterEqual(x, 0)
        self.assertGreaterEqual(y, 0)


class CombatTest(ServerTestCase):
    def test_holding_fire_shoots_four_times_per_second(self):
        client = self.joined_client()
        packets = client.play(1.0, proto.BUTTON_FIRE)
        shots = [proto.parse_spawn(p)[1] for p in packets if p.type == proto.ENTITY_SPAWN]
        shots = [e for e in shots if e.kind == proto.KIND_PLAYER_PROJECTILE]

        self.assertGreaterEqual(len(shots), 3, "fire cooldown is 0.25 s")
        self.assertLessEqual(len(shots), 5)
        for shot in shots:
            self.assertEqual(shot.owner, client.player_id)
            self.assertEqual(shot.health, 0)

    def test_projectiles_move_forward(self):
        client = self.joined_client()
        client.play(0.1, proto.BUTTON_FIRE)
        received = snapshots(client.play(0.3))
        positions = {}
        for snapshot in received:
            for entity in snapshot.entities:
                if entity.kind == proto.KIND_PLAYER_PROJECTILE:
                    positions.setdefault(entity.entity_id, []).append(entity.x)

        self.assertTrue(positions, "the projectile never appeared in a snapshot")
        for xs in positions.values():
            self.assertEqual(xs, sorted(xs))
            self.assertGreater(xs[-1], xs[0])

    def test_enemies_spawn(self):
        client = self.joined_client()
        packets = client.play(8.0)
        enemies = {e.entity_id for s in snapshots(packets) for e in s.entities if e.kind == proto.KIND_ENEMY}

        self.assertTrue(enemies, "no enemy appeared in 8 seconds")


class ReplicationTest(ServerTestCase):
    def test_every_server_packet_follows_the_rfc(self):
        clients = [self.joined_client() for _ in range(2)]
        packets = []
        deadline = time.monotonic() + 8.0
        buttons = [proto.BUTTON_FIRE | proto.BUTTON_DOWN, proto.BUTTON_FIRE | proto.BUTTON_RIGHT]
        while time.monotonic() < deadline:
            for client, held in zip(clients, buttons):
                client.send_input(held)
                packets += client.drain(0.025)

        self.assertTrue(packets)
        last_tick = 0
        for packet in packets:
            if packet.type in proto.EXPECTED_PAYLOAD_SIZE:
                self.assertEqual(len(packet.payload), proto.EXPECTED_PAYLOAD_SIZE[packet.type], hex(packet.type))
            if packet.type != proto.WORLD_SNAPSHOT:
                continue
            snapshot = proto.parse_snapshot(packet)
            self.assertEqual(packet.session_id, 0)
            self.assertLessEqual(len(snapshot.entities), 64)
            self.assertEqual(snapshot.connected_mask & 0xF0, 0)
            self.assertEqual(snapshot.alive_mask & ~snapshot.connected_mask, 0, "alive bit set for a disconnected player")
            ids = [e.entity_id for e in snapshot.entities]
            self.assertEqual(len(ids), len(set(ids)), "duplicate entity ID in a snapshot")
            for entity in snapshot.entities:
                self.assertIn(entity.kind, (1, 2, 3, 4))
                if entity.kind == proto.KIND_PLAYER:
                    self.assertEqual(entity.variant, entity.owner)
                if entity.kind in (proto.KIND_ENEMY, proto.KIND_ENEMY_PROJECTILE):
                    self.assertEqual(entity.owner, 255)
            last_tick = max(last_tick, snapshot.server_tick)
        self.assertGreater(last_tick, 0)

    def test_both_players_see_the_same_world(self):
        first, second = self.joined_client(), self.joined_client()
        first.play(0.3, proto.BUTTON_DOWN)
        deadline = time.monotonic() + 0.5
        while time.monotonic() < deadline:
            first.send_input(0)
            second.send_input(0)
            first.drain(0.025)
            second.drain(0.025)
        seen_by_first = {s.server_tick: s for s in snapshots(first.received)}
        seen_by_second = {s.server_tick: s for s in snapshots(second.received)}
        common = sorted(set(seen_by_first) & set(seen_by_second))

        self.assertTrue(common, "the two clients never received the same snapshot")
        tick = common[-1]
        self.assertEqual(seen_by_first[tick], seen_by_second[tick])
        self.assertEqual(bin(seen_by_first[tick].connected_mask).count("1"), 2)


if __name__ == "__main__":
    unittest.main()
