/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Protocol function and structs definition
*/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "game/GameTypes.hpp"

namespace rtype::network {

static constexpr std::size_t HEADER_SIZE = 16;
static constexpr std::size_t MAX_DATAGRAM_SIZE = 1200;
static constexpr std::array<std::uint8_t, 4> MAGIC{'R', 'T', 'Y', 'P'};
static constexpr std::uint8_t VERSION = 1;
static constexpr std::uint32_t HALF_SEQUENCE = 2147483648;
static constexpr std::size_t MAX_SNAPSHOT_ENTITIES = 64;

enum class PacketType : std::uint8_t {
	JoinRequest = 0x01,
	JoinAccepted = 0x02,
	JoinRejected = 0x03,
	Disconnect = 0x04,
	PlayerLeft = 0x05,
	PlayerInput = 0x10,
	WorldSnapshot = 0x20,
	EntitySpawn = 0x21,
	EntityDestroy = 0x22,
	EntityDamage = 0x23,
	PlayerDeath = 0x24,
	Error = 0x67,
};

enum class ErrorCode : std::uint8_t {
	UnsupportedVersion = 1,
	InvalidLength = 2,
	UnknownType = 3,
	WrongDirection = 4,
	InvalidSession = 5,
	InvalidField = 6,
};

struct Header {
	PacketType type;
	std::uint16_t payloadSize;
	std::uint32_t sequence;
	std::uint32_t sessionId;
};

struct ParsedHeader {
	std::optional<Header> header;
	std::optional<ErrorCode> error;
	std::uint8_t offendingType;
	std::uint32_t offendingSequence;
};

ParsedHeader parseClientHeader(const std::vector<std::uint8_t>& bytes);
std::optional<Header> parseServerHeader(const std::vector<std::uint8_t>& bytes);
std::optional<ErrorCode> validateClientPayload(const Header& header, const std::vector<std::uint8_t>& bytes);
std::optional<game::EntityType> decodeEntityType(std::uint8_t wireKind);
std::vector<std::uint8_t> makePacket(PacketType type, std::uint32_t sequence,
	std::uint32_t sessionId, const std::vector<std::uint8_t>& payload);
std::vector<std::uint8_t> makeError(ErrorCode code, const ParsedHeader& rejected);
bool isNewerSequence(std::uint32_t candidate, std::uint32_t previous);
void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value);
void appendU32(std::vector<std::uint8_t>& bytes, std::uint32_t value);
void appendU64(std::vector<std::uint8_t>& bytes, std::uint64_t value);
std::uint32_t readU32(const std::vector<std::uint8_t>& bytes, std::size_t offset);
std::uint64_t readU64(const std::vector<std::uint8_t>& bytes, std::size_t offset);

}
