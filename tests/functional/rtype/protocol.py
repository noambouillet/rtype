"""
R-Type UDP protocol, written from docs/RFC.md.

This module is deliberately independent from the C++ code: the functional tests
check that the binaries follow the RFC, so they must not reuse the code under
test to build or read packets.
"""

import struct
from dataclasses import dataclass, field

MAGIC = b"RTYP"
VERSION = 1
HEADER_SIZE = 16
MAX_DATAGRAM_SIZE = 1200
HEADER_FORMAT = ">4sBBHII"

JOIN_REQUEST = 0x01
JOIN_ACCEPTED = 0x02
JOIN_REJECTED = 0x03
DISCONNECT = 0x04
PLAYER_LEFT = 0x05
PLAYER_INPUT = 0x10
WORLD_SNAPSHOT = 0x20
ENTITY_SPAWN = 0x21
ENTITY_DESTROY = 0x22
ENTITY_DAMAGE = 0x23
PLAYER_DEATH = 0x24
ERROR = 0x67

ERR_UNSUPPORTED_VERSION = 1
ERR_INVALID_LENGTH = 2
ERR_UNKNOWN_TYPE = 3
ERR_WRONG_DIRECTION = 4
ERR_INVALID_SESSION = 5
ERR_INVALID_FIELD = 6

REJECT_SERVER_FULL = 1
LEFT_DISCONNECT = 1
LEFT_TIMEOUT = 2

BUTTON_UP = 1 << 0
BUTTON_DOWN = 1 << 1
BUTTON_LEFT = 1 << 2
BUTTON_RIGHT = 1 << 3
BUTTON_FIRE = 1 << 4

KIND_PLAYER = 1
KIND_ENEMY = 2
KIND_PLAYER_PROJECTILE = 3
KIND_ENEMY_PROJECTILE = 4

ENTITY_FORMAT = ">IBBBBii"
ENTITY_SIZE = 16


@dataclass
class Packet:
    type: int
    sequence: int
    session_id: int
    payload: bytes


@dataclass
class Entity:
    entity_id: int
    kind: int
    owner: int
    variant: int
    health: int
    x: int
    y: int


@dataclass
class Snapshot:
    server_tick: int
    connected_mask: int
    alive_mask: int
    entities: list = field(default_factory=list)

    def players(self):
        return [e for e in self.entities if e.kind == KIND_PLAYER]

    def player(self, player_id):
        for entity in self.players():
            if entity.owner == player_id:
                return entity
        return None


def build(packet_type, sequence=0, session_id=0, payload=b"", *,
          magic=MAGIC, version=VERSION, payload_size=None):
    """Builds a datagram. Every header field can be forced to craft invalid packets."""
    if payload_size is None:
        payload_size = len(payload)
    header = struct.pack(HEADER_FORMAT, magic, version, packet_type,
                         payload_size, sequence, session_id)
    return header + payload


def join_request(nonce):
    return build(JOIN_REQUEST, 0, 0, struct.pack(">Q", nonce))


def player_input(session_id, sequence, buttons):
    return build(PLAYER_INPUT, sequence, session_id, bytes([buttons]))


def disconnect(session_id, sequence):
    return build(DISCONNECT, sequence, session_id)


def parse(datagram):
    """Parses a server datagram, or returns None when it breaks the header rules."""
    if len(datagram) < HEADER_SIZE or len(datagram) > MAX_DATAGRAM_SIZE:
        return None
    magic, version, packet_type, size, sequence, session = struct.unpack_from(HEADER_FORMAT, datagram)
    if magic != MAGIC or version != VERSION or len(datagram) != HEADER_SIZE + size:
        return None
    return Packet(packet_type, sequence, session, datagram[HEADER_SIZE:])


def parse_join_reply(packet):
    """Returns (nonce, player_id_or_reason)."""
    return struct.unpack(">QB", packet.payload)


def parse_player_left(packet):
    """Returns (server_tick, player_id, reason)."""
    return struct.unpack(">QBB", packet.payload)


def parse_error(packet):
    """Returns (code, offending_type, offending_sequence, detail)."""
    return struct.unpack(">BBIB", packet.payload)


def parse_entity(data, offset=0):
    return Entity(*struct.unpack_from(ENTITY_FORMAT, data, offset))


def parse_snapshot(packet):
    tick, connected, alive, count = struct.unpack_from(">QBBB", packet.payload)
    if len(packet.payload) != 11 + ENTITY_SIZE * count:
        raise ValueError(f"snapshot announces {count} entities but has {len(packet.payload)} payload bytes")
    entities = [parse_entity(packet.payload, 11 + ENTITY_SIZE * i) for i in range(count)]
    return Snapshot(tick, connected, alive, entities)


def parse_spawn(packet):
    """Returns (server_tick, Entity)."""
    (tick,) = struct.unpack_from(">Q", packet.payload)
    return tick, parse_entity(packet.payload, 8)


EXPECTED_PAYLOAD_SIZE = {
    JOIN_ACCEPTED: 9,
    JOIN_REJECTED: 9,
    PLAYER_LEFT: 10,
    ENTITY_SPAWN: 24,
    ENTITY_DESTROY: 21,
    ENTITY_DAMAGE: 13,
    PLAYER_DEATH: 21,
    ERROR: 7,
}
