"""Invalid datagrams get the ERROR the RFC asks for, and garbage never crashes the server."""

import random
import unittest

from rtype import protocol as proto
from rtype.harness import ServerTestCase


class ErrorResponseTest(ServerTestCase):
    def expect_error(self, client, datagram, code, detail=0):
        client.send(datagram)
        packet = client.wait_for(proto.ERROR, timeout=0.5)
        self.assertIsNotNone(packet, "no ERROR received")
        self.assertEqual(packet.sequence, 0)
        self.assertEqual(packet.session_id, 0)
        got_code, offending_type, offending_sequence, got_detail = proto.parse_error(packet)
        self.assertEqual(got_code, code)
        self.assertEqual(offending_type, datagram[5])
        self.assertEqual(offending_sequence, int.from_bytes(datagram[8:12], "big"))
        self.assertEqual(got_detail, detail)

    def test_each_invalid_packet_gets_its_error_code(self):
        player = self.joined_client()
        session = player.session_id
        nonce = (42).to_bytes(8, "big")
        cases = {
            "unsupported version": (proto.build(proto.JOIN_REQUEST, 0, 0, nonce, version=2),
                                    proto.ERR_UNSUPPORTED_VERSION, 1),
            "declared length too long": (proto.build(proto.PLAYER_INPUT, 7, session, b"\x00", payload_size=5),
                                         proto.ERR_INVALID_LENGTH, 0),
            "datagram over 1200 bytes": (proto.build(proto.JOIN_REQUEST, 0, 0, bytes(1200)),
                                         proto.ERR_INVALID_LENGTH, 0),
            "unknown type": (proto.build(0x50, 3, 0), proto.ERR_UNKNOWN_TYPE, 0),
            "server-only packet": (proto.build(proto.WORLD_SNAPSHOT, 3, 0, bytes(11)),
                                   proto.ERR_WRONG_DIRECTION, 0),
            "unknown session": (proto.player_input(session ^ 0xFFFFFFFF, 3, 0), proto.ERR_INVALID_SESSION, 0),
            "join payload too short": (proto.build(proto.JOIN_REQUEST, 0, 0, nonce[:7]), proto.ERR_INVALID_LENGTH, 0),
            "join with a sequence": (proto.build(proto.JOIN_REQUEST, 9, 0, nonce), proto.ERR_INVALID_FIELD, 0),
            "input with sequence 0": (proto.player_input(session, 0, 0), proto.ERR_INVALID_FIELD, 0),
            "input with reserved bits": (proto.player_input(session, 11, 0x20), proto.ERR_INVALID_FIELD, 0),
            "input with two bytes": (proto.build(proto.PLAYER_INPUT, 12, session, b"\x00\x00"),
                                     proto.ERR_INVALID_LENGTH, 0),
            "disconnect with payload": (proto.build(proto.DISCONNECT, 13, session, b"\x00"),
                                        proto.ERR_INVALID_LENGTH, 0),
        }
        for name, (datagram, code, detail) in cases.items():
            with self.subTest(name):
                self.expect_error(player, datagram, code, detail)

    def test_first_failure_is_reported(self):
        client = self.client()
        datagram = proto.build(0x50, 0, 0, b"", version=7, payload_size=99)
        self.expect_error(client, datagram, proto.ERR_UNSUPPORTED_VERSION, 1)

    def test_no_answer_to_short_foreign_or_error_datagrams(self):
        client = self.client()
        for name, datagram in {
            "short datagram": b"RTYP\x01\x01",
            "wrong magic": proto.build(proto.JOIN_REQUEST, 0, 0, bytes(8), magic=b"QUAK"),
            "client ERROR": proto.build(proto.ERROR, 0, 0, bytes(7)),
            "empty datagram": b"",
        }.items():
            with self.subTest(name):
                client.send(datagram)
                self.assertIsNone(client.recv(0.3), "the server must stay silent")

    def test_invalid_packet_does_not_end_the_session(self):
        client = self.joined_client()
        client.send(proto.player_input(client.session_id, client.next_sequence(), 0xFF))
        client.play(0.3)

        self.assertTrue(client.wait_for(proto.WORLD_SNAPSHOT, timeout=0.5))


class FuzzTest(ServerTestCase):
    def random_datagrams(self, rng, count):
        types = [0x01, 0x02, 0x04, 0x05, 0x10, 0x20, 0x24, 0x67, 0x99]
        for _ in range(count):
            if rng.random() < 0.3:
                yield rng.randbytes(rng.randint(0, 64))
                continue
            payload = rng.randbytes(rng.choice([0, 1, 7, 8, 9, 50]))
            yield proto.build(rng.choice(types), rng.getrandbits(32) if rng.random() < 0.5 else 0,
                              rng.getrandbits(32) if rng.random() < 0.5 else 0, payload,
                              version=rng.choice([1, 1, 1, 0, 2, 255]),
                              payload_size=rng.choice([len(payload)] * 3 + [rng.getrandbits(16)]))

    def test_server_survives_random_datagrams(self):
        rng = random.Random(1234)
        player = self.joined_client()
        attacker = self.client()
        for index, datagram in enumerate(self.random_datagrams(rng, 3000)):
            attacker.send(datagram)
            if index % 100 == 0:
                player.send_input(0)
                attacker.drain(0.005)

        self.assertTrue(self.server.is_running(), self.server.log())
        self.assertTrue(player.wait_for(proto.WORLD_SNAPSHOT, timeout=1.0), "the server stopped replying")
        player.play(0.2)
        last = proto.parse_snapshot(player.wait_for(proto.WORLD_SNAPSHOT, timeout=0.5))
        self.assertTrue(last.connected_mask & (1 << player.player_id), "the fuzzing dropped a real player")

    def test_server_survives_a_join_flood(self):
        attacker = self.client()
        for nonce in range(2000):
            attacker.send(proto.join_request(nonce))
        attacker.drain(0.5)

        self.assertTrue(self.server.is_running())
        replies = [p for p in attacker.received if p.type in (proto.JOIN_ACCEPTED, proto.JOIN_REJECTED)]
        self.assertEqual(sum(p.type == proto.JOIN_ACCEPTED for p in replies), 4, "only 4 slots exist")


if __name__ == "__main__":
    unittest.main()
