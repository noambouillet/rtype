/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Protocol implementation
*/

#include "Protocol.hpp"

#include <algorithm>
#include <array>

namespace rtype::network {

/// Accepts only entity kinds defined by the game before converting a wire byte.
std::optional<game::EntityType> decodeEntityType(std::uint8_t wireKind)
{
	const auto kind = static_cast<game::EntityType>(wireKind);
	switch (kind) {
		case game::EntityType::Player:
		case game::EntityType::Enemy:
		case game::EntityType::PlayerProjectile:
		case game::EntityType::EnemyProjectile:
			return kind;
		default:
			return std::nullopt;
	}
}

static bool isKnownType(std::uint8_t type)
{
	switch (static_cast<PacketType>(type)) {
		case PacketType::JoinRequest:
		case PacketType::JoinAccepted:
		case PacketType::JoinRejected:
		case PacketType::Disconnect:
		case PacketType::PlayerLeft:
		case PacketType::PlayerInput:
		case PacketType::WorldSnapshot:
		case PacketType::EntitySpawn:
		case PacketType::EntityDestroy:
		case PacketType::EntityDamage:
		case PacketType::PlayerDeath:
		case PacketType::Error:
			return true;
	}
	return false;
}

/// Appends a 16-bit integer in network byte order.
void appendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value)
{
	bytes.push_back(static_cast<std::uint8_t>(value >> 8));
	bytes.push_back(static_cast<std::uint8_t>(value));
}

/// Appends a 32-bit integer in network byte order.
void appendU32(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
	for (int shift = 24; shift >= 0; shift -= 8) {
		bytes.push_back(static_cast<std::uint8_t>(value >> shift));
	}
}

/// Appends a 64-bit integer in network byte order.
void appendU64(std::vector<std::uint8_t>& bytes, std::uint64_t value)
{
	for (int shift = 56; shift >= 0; shift -= 8) {
		bytes.push_back(static_cast<std::uint8_t>(value >> shift));
	}
}

/// Reads a network-order 32-bit integer from previously validated bytes.
std::uint32_t readU32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
	return (static_cast<std::uint32_t>(bytes[offset]) << 24) |
		   (static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
		   (static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
		   static_cast<std::uint32_t>(bytes[offset + 3]);
}

/// Reads a network-order 64-bit integer from previously validated bytes.
std::uint64_t readU64(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
	return (static_cast<std::uint64_t>(readU32(bytes, offset)) << 32) |
		   readU32(bytes, offset + 4);
}

/// Validates the common header and client-to-server packet direction.
/// Invalid magic and short datagrams yield neither a header nor an error.
ParsedHeader parseClientHeader(const std::vector<std::uint8_t>& bytes)
{
	ParsedHeader result{};
	if (bytes.size() < HEADER_SIZE ||
		!std::equal(MAGIC.begin(), MAGIC.end(), bytes.begin())) {
		return result;
	}

	result.offendingType = bytes[5];
	result.offendingSequence = readU32(bytes, 8);
	if (bytes[5] == static_cast<std::uint8_t>(PacketType::Error)) {
		return result;
	}
	if (bytes[4] != VERSION) {
		result.error = ErrorCode::UnsupportedVersion;
	} else if (bytes.size() > MAX_DATAGRAM_SIZE || bytes.size() != HEADER_SIZE + (static_cast<std::uint16_t>(bytes[6]) << 8 | bytes[7])) {
		result.error = ErrorCode::InvalidLength;
	} else if (!isKnownType(bytes[5])) {
		result.error = ErrorCode::UnknownType;
	} else if (bytes[5] != static_cast<std::uint8_t>(PacketType::JoinRequest) &&
			   bytes[5] != static_cast<std::uint8_t>(PacketType::Disconnect) &&
			   bytes[5] != static_cast<std::uint8_t>(PacketType::PlayerInput)) {
		// If the client sends an ERROR packet, he never receives an ERROR in response.
		if (bytes[5] != static_cast<std::uint8_t>(PacketType::Error)) {
			result.error = ErrorCode::WrongDirection;
		}
	} else {
		result.header = Header{
			.type = static_cast<PacketType>(bytes[5]),
			.payloadSize = static_cast<std::uint16_t>(bytes[6] << 8 | bytes[7]),
			.sequence = result.offendingSequence,
			.sessionId = readU32(bytes, 12)};
	}
	return result;
}

/// Checks the implemented join response fields.
static bool validJoinReply(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	if (header.payloadSize != 9 || header.sequence != 0) {
		return false;
	}
	if (header.type == PacketType::JoinAccepted) {
		return header.sessionId != 0 && bytes[HEADER_SIZE + 8] <= 3;
	}
	return header.sessionId == 0 && bytes[HEADER_SIZE + 8] == 1;
}

/// Checks the implemented session notice fields.
static bool validSessionNotice(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	return header.payloadSize == 10 && header.sequence != 0 && header.sessionId == 0 &&
		   bytes[HEADER_SIZE + 8] <= 3 && (bytes[HEADER_SIZE + 9] == 1 || bytes[HEADER_SIZE + 9] == 2);
}

/// Checks one entity record shared by snapshots and spawn events.
static bool validEntityRecord(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
	const auto decoded = decodeEntityType(bytes[offset + 4]);
	if (!decoded) {
		return false;
	}
	const game::EntityType kind = *decoded;
	const std::uint8_t owner = bytes[offset + 5];
	const std::uint8_t variant = bytes[offset + 6];
	const std::uint8_t health = bytes[offset + 7];
	if (kind == game::EntityType::Player) {
		return owner <= 3 && variant == owner && health > 0;
	}
	if (kind == game::EntityType::PlayerProjectile) {
		return owner <= 3 && health == 0;
	}
	return owner == game::NO_OWNER && (kind == game::EntityType::Enemy ? health > 0 : health == 0);
}

/// Checks the masks, record count, and unique IDs in a world snapshot.
static bool validWorldSnapshot(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	if (header.sequence == 0 || header.sessionId != 0 || header.payloadSize < 11) {
		return false;
	}
	const std::uint8_t connectedMask = bytes[HEADER_SIZE + 8];
	const std::uint8_t aliveMask = bytes[HEADER_SIZE + 9];
	const std::size_t entityCount = bytes[HEADER_SIZE + 10];
	if ((connectedMask & 0xF0) != 0 || (aliveMask & 0xF0) != 0 || (aliveMask & connectedMask) != aliveMask ||
		entityCount > 64 || header.payloadSize != 11 + entityCount * 16) {
		return false;
	}

	std::array<std::uint32_t, 64> entityIds{};
	for (std::size_t index = 0; index < entityCount; index++) {
		const std::size_t recordOffset = HEADER_SIZE + 11 + index * 16;
		if (!validEntityRecord(bytes, recordOffset)) {
			return false;
		}
		const std::uint32_t entityId = readU32(bytes, recordOffset);
		for (std::size_t previous = 0; previous < index; previous++) {
			if (entityIds[previous] == entityId) {
				return false;
			}
		}
		entityIds[index] = entityId;
	}
	return true;
}

/// Checks event sizes and values before the client reads their payloads.
static bool validWorldEvent(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	if (header.sequence == 0 || header.sessionId != 0 || header.payloadSize < 8 || readU64(bytes, HEADER_SIZE) == 0) {
		return false;
	}
	const std::size_t offset = HEADER_SIZE + 8;
	switch (header.type) {
		case PacketType::EntitySpawn:
			return header.payloadSize == 24 && validEntityRecord(bytes, offset);
		case PacketType::EntityDestroy:
			return header.payloadSize == 21 && decodeEntityType(bytes[offset + 4]) &&
				   decodeEntityType(bytes[offset + 4]) != game::EntityType::Player;
		case PacketType::EntityDamage:
			return header.payloadSize == 13 && bytes[offset + 4] > 0;
		case PacketType::PlayerDeath:
			return header.payloadSize == 21 && bytes[offset] <= 3;
		default:
			return false;
	}
}

/// Checks the implemented protocol error fields.
static bool validErrorNotice(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	return header.payloadSize == 7 && header.sequence == 0 && header.sessionId == 0 &&
		   bytes[HEADER_SIZE] >= 1 && bytes[HEADER_SIZE] <= 6 &&
		   bytes[HEADER_SIZE + 6] == (bytes[HEADER_SIZE] == 1 ? VERSION : 0);
}

/// Validates the fields specific to implemented server packet types.
static bool validServerPayload(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	switch (header.type) {
		case PacketType::JoinAccepted:
		case PacketType::JoinRejected:
			return validJoinReply(header, bytes);
		case PacketType::PlayerLeft:
			return validSessionNotice(header, bytes);
		case PacketType::WorldSnapshot:
			return validWorldSnapshot(header, bytes);
		case PacketType::EntitySpawn:
		case PacketType::EntityDestroy:
		case PacketType::EntityDamage:
		case PacketType::PlayerDeath:
			return validWorldEvent(header, bytes);
		case PacketType::Error:
			return validErrorNotice(header, bytes);
		default:
			return false;
	}
}

/// Validates the common header and the implemented server-to-client packets.
std::optional<Header> parseServerHeader(const std::vector<std::uint8_t>& bytes)
{
	if (bytes.size() < HEADER_SIZE || bytes.size() > MAX_DATAGRAM_SIZE ||
		!std::equal(MAGIC.begin(), MAGIC.end(), bytes.begin()) || bytes[4] != VERSION) {
		return std::nullopt;
	}
	const auto payloadSize = static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[6]) << 8) | bytes[7]);
	if (bytes.size() != HEADER_SIZE + payloadSize) {
		return std::nullopt;
	}
	Header header;
	header.type = static_cast<PacketType>(bytes[5]);
	header.payloadSize = payloadSize;
	header.sequence = readU32(bytes, 8);
	header.sessionId = readU32(bytes, 12);
	if (!validServerPayload(header, bytes)) {
		return std::nullopt;
	}
	return header;
}

/// Validates the payload and field ranges after the caller checks the session.
std::optional<ErrorCode> validateClientPayload(const Header& header, const std::vector<std::uint8_t>& bytes)
{
	if (header.type == PacketType::JoinRequest) {
		if (header.payloadSize != 8) {
			return ErrorCode::InvalidLength;
		}
		if (header.sequence != 0 || header.sessionId != 0) {
			return ErrorCode::InvalidField;
		}
	} else if (header.type == PacketType::Disconnect) {
		if (header.payloadSize != 0) {
			return ErrorCode::InvalidLength;
		}
		if (header.sequence == 0) {
			return ErrorCode::InvalidField;
		}
	} else if (header.type == PacketType::PlayerInput) {
		if (header.payloadSize != 1) {
			return ErrorCode::InvalidLength;
		}
		if (header.sequence == 0 || bytes[HEADER_SIZE] > game::MAX_BUTTON_STATE) {
			return ErrorCode::InvalidField;
		}
	}
	return std::nullopt;
}

/// Builds a protocol datagram, or an empty vector if it exceeds the MTU.
std::vector<std::uint8_t> makePacket(PacketType type, std::uint32_t sequence,
	std::uint32_t sessionId, const std::vector<std::uint8_t>& payload)
{
	if (payload.size() > MAX_DATAGRAM_SIZE - HEADER_SIZE) {
		return {};
	}
	std::vector<std::uint8_t> bytes;
	bytes.reserve(HEADER_SIZE + payload.size());
	bytes.insert(bytes.end(), MAGIC.begin(), MAGIC.end());
	bytes.push_back(VERSION);
	bytes.push_back(static_cast<std::uint8_t>(type));
	appendU16(bytes, static_cast<std::uint16_t>(payload.size()));
	appendU32(bytes, sequence);
	appendU32(bytes, sessionId);
	bytes.insert(bytes.end(), payload.begin(), payload.end());
	return bytes;
}

/// Builds the protocol error response for a rejected client datagram.
std::vector<std::uint8_t> makeError(ErrorCode code, const ParsedHeader& rejected)
{
	std::vector<std::uint8_t> payload{static_cast<std::uint8_t>(code), rejected.offendingType};
	appendU32(payload, rejected.offendingSequence);
	payload.push_back(code == ErrorCode::UnsupportedVersion ? VERSION : 0);
	return makePacket(PacketType::Error, 0, 0, payload);
}

/// Returns whether "candidate" follows "previous" in the RFC sequence order.
bool isNewerSequence(std::uint32_t candidate, std::uint32_t previous)
{
	return candidate != previous && candidate - previous < HALF_SEQUENCE;
}

}
