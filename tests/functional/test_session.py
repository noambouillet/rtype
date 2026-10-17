"""Joining, leaving and timing out, as described in the "Joining and sessions" part of the RFC."""

import time
import unittest

from rtype import protocol as proto
from rtype.harness import ServerTestCase, snapshots


class JoinTest(ServerTestCase):
    def test_join_is_accepted(self):
        client = self.client()
        reply = client.join(nonce=0x0123456789ABCDEF)

        self.assertIsNotNone(reply, "no answer to JOIN_REQUEST")
        self.assertEqual(reply.type, proto.JOIN_ACCEPTED)
        nonce, player_id = proto.parse_join_reply(reply)
        self.assertEqual(nonce, 0x0123456789ABCDEF, "the nonce must be echoed")
        self.assertEqual(player_id, 0, "the first player gets ID 0")
        self.assertEqual(reply.sequence, 0)
        self.assertNotEqual(reply.session_id, 0)

    def test_first_snapshot_shows_the_new_player(self):
        client = self.joined_client()
        client.send_input(0)
        packet = client.wait_for(proto.WORLD_SNAPSHOT, timeout=0.5)

        self.assertIsNotNone(packet, "no WORLD_SNAPSHOT after joining")
        snapshot = proto.parse_snapshot(packet)
        bit = 1 << client.player_id
        self.assertTrue(snapshot.connected_mask & bit)
        self.assertTrue(snapshot.alive_mask & bit)
        ship = snapshot.player(client.player_id)
        self.assertIsNotNone(ship, "the player's ship is missing from the snapshot")
        self.assertEqual(ship.variant, client.player_id)

    def test_snapshots_arrive_every_50ms_with_increasing_ticks(self):
        client = self.joined_client()
        received = snapshots(client.play(2.0))

        self.assertGreaterEqual(len(received), 30, "expected about 40 snapshots in 2 seconds")
        self.assertLessEqual(len(received), 50)
        ticks = [s.server_tick for s in received]
        self.assertEqual(ticks, sorted(set(ticks)), "snapshot ticks must strictly increase")

    def test_repeated_join_with_same_nonce_keeps_the_session(self):
        client = self.joined_client()
        first_session, first_player = client.session_id, client.player_id
        reply = client.join(nonce=client.nonce)

        self.assertEqual(reply.type, proto.JOIN_ACCEPTED)
        self.assertEqual(client.session_id, first_session)
        self.assertEqual(client.player_id, first_player)
        last = snapshots(client.play(0.3))[-1]
        self.assertEqual(bin(last.connected_mask).count("1"), 1, "a repeated join must not add a player")

    def test_four_players_then_server_full(self):
        players = [self.joined_client() for _ in range(4)]

        self.assertEqual(sorted(p.player_id for p in players), [0, 1, 2, 3])
        self.assertEqual(len({p.session_id for p in players}), 4, "session IDs must be unique")
        fifth = self.client().join()
        self.assertIsNotNone(fifth)
        self.assertEqual(fifth.type, proto.JOIN_REJECTED)
        self.assertEqual(proto.parse_join_reply(fifth)[1], proto.REJECT_SERVER_FULL)
        self.assertEqual(fifth.session_id, 0)


class LeaveTest(ServerTestCase):
    def test_disconnect_notifies_the_other_players(self):
        stays, leaves = self.joined_client(), self.joined_client()
        leaves.leave()
        packet = stays.wait_for(proto.PLAYER_LEFT, timeout=1.0)

        self.assertIsNotNone(packet, "no PLAYER_LEFT after DISCONNECT")
        _, player_id, reason = proto.parse_player_left(packet)
        self.assertEqual(player_id, leaves.player_id)
        self.assertEqual(reason, proto.LEFT_DISCONNECT)
        last = snapshots(stays.play(0.3))[-1]
        self.assertFalse(last.connected_mask & (1 << leaves.player_id))
        self.assertIsNone(last.player(leaves.player_id), "the ship of a player who left must be removed")

    def test_free_slot_is_reused(self):
        players = [self.joined_client() for _ in range(4)]
        players[2].leave()
        time.sleep(0.1)

        newcomer = self.joined_client()
        self.assertEqual(newcomer.player_id, players[2].player_id)
        self.assertNotEqual(newcomer.session_id, players[2].session_id, "session IDs are never reused")

    def test_rejoining_with_a_new_nonce_gives_a_new_session(self):
        client = self.joined_client()
        old_session = client.session_id
        client.leave()
        time.sleep(0.1)
        reply = client.join(nonce=client.nonce + 1)

        self.assertEqual(reply.type, proto.JOIN_ACCEPTED)
        self.assertNotEqual(client.session_id, old_session)

    def test_silent_player_times_out_after_5_seconds(self):
        stays, silent = self.joined_client(), self.joined_client()
        start = time.monotonic()
        packets = stays.play(6.5)
        left = [p for p in packets if p.type == proto.PLAYER_LEFT]

        self.assertEqual(len(left), 1, "expected exactly one PLAYER_LEFT")
        _, player_id, reason = proto.parse_player_left(left[0])
        self.assertEqual(player_id, silent.player_id)
        self.assertEqual(reason, proto.LEFT_TIMEOUT)
        self.assertGreaterEqual(time.monotonic() - start, 5.0)

    def test_active_player_does_not_time_out(self):
        client = self.joined_client()
        packets = client.play(6.0)

        self.assertFalse([p for p in packets if p.type == proto.PLAYER_LEFT])
        self.assertTrue(snapshots(packets)[-1].connected_mask & (1 << client.player_id))


if __name__ == "__main__":
    unittest.main()
