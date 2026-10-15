/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Server sessions and packet translation
*/

#include "ServerNetworkService.hpp"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#ifdef _WIN32

// clang-format off
// Clang wants the windows header after the bcrypt one, but this doesnt compile.
// So we put "clang-format off" to wrestle with clang's ignorance.
#include <windows.h>
#include <bcrypt.h>
// clang-format on

#else

#include <sys/random.h>

#endif

#include "utils/RTypeError.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype {

/// Draws a session ID from the operating system's cryptographic random source.
#ifdef _WIN32

static std::uint32_t randomSessionId()
{
	std::uint32_t id = 0;

	if (BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(&id), sizeof(id), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) {
		throw NetworkError("Unable to generate a session ID");
	}
	return id;
}

#else

static std::uint32_t randomSessionId()
{
	std::uint32_t id = 0;
	std::size_t filled = 0;

	while (filled < sizeof(id)) {
		const ssize_t received = getrandom(reinterpret_cast<std::uint8_t*>(&id) + filled, sizeof(id) - filled, 0);
		if (received < 0 && errno == EINTR) {
			continue;
		}
		if (received <= 0) {
			throw NetworkError("Unable to generate a session ID");
		}
		filled += static_cast<std::size_t>(received);
	}
	return id;
}

#endif

/// Encodes a world coordinate as a signed 1/16-pixel value in network byte order. (So need to divide by 16.0F on the client side to get the original float value)
static std::uint32_t encodePosition(float position)
{
	const double scaled = static_cast<double>(position) * 16.0F;
	const auto minimum = static_cast<double>(std::numeric_limits<std::int32_t>::min());
	const auto maximum = static_cast<double>(std::numeric_limits<std::int32_t>::max());
	const auto fixed = static_cast<std::int32_t>(std::lround(std::clamp(scaled, minimum, maximum)));
	return static_cast<std::uint32_t>(fixed);
}

/// Creates the game server: binds the UDP port and starts an empty world.
ServerNetworkService::ServerNetworkService(network::IServerTransport& transport)
	: _network(transport), _sessions{}, _issuedSessionIds{}, _tick(0)
{
}

/// Appends one 16-byte entity record to the snapshot payload, multi-byte fields in big-endian:
static void appendEntityRecord(std::vector<std::uint8_t>& payload, const GameWorld::EntityState& entity)
{
	network::appendU32(payload, entity.id);
	payload.push_back(static_cast<std::uint8_t>(entity.type));
	payload.push_back(entity.owner);
	payload.push_back(entity.variant);
	payload.push_back(entity.health);
	network::appendU32(payload, encodePosition(entity.x));
	network::appendU32(payload, encodePosition(entity.y));
}

/// Builds the documented full snapshot payload from the world's replicated entities.
std::vector<std::uint8_t> ServerNetworkService::buildSnapshot(const std::vector<GameWorld::EntityState>& entities,
	std::uint8_t connectedMask, std::uint8_t aliveMask)
{
	const std::size_t count = std::min(entities.size(), network::MAX_SNAPSHOT_ENTITIES);
	std::vector<std::uint8_t> payload;
	payload.reserve(SNAPSHOT_HEADER_SIZE + count * ENTITY_RECORD_SIZE);
	network::appendU64(payload, _tick);
	payload.push_back(connectedMask);
	payload.push_back(aliveMask);
	payload.push_back(static_cast<std::uint8_t>(count));
	for (std::size_t index = 0; index < count; index++) {
		appendEntityRecord(payload, entities[index]);
	}
	return payload;
}

/// Encodes and broadcasts one full authoritative world snapshot.
void ServerNetworkService::broadcastSnapshot(const std::vector<GameWorld::EntityState>& entities,
	std::uint8_t connectedMask, std::uint8_t aliveMask)
{
	broadcast(network::PacketType::WorldSnapshot, buildSnapshot(entities, connectedMask, aliveMask));
}

/// Processes queued packets and returns the current inputs for the game world.
GameWorld::Input ServerNetworkService::tick()
{
	_tick++;
	const auto now = Clock::now();

	for (const auto& datagram : _network.receivePending()) {
		handleDatagram(datagram, now);
	}
	return collectInput(now);
}

/// Validates one client datagram and updates its session.
void ServerNetworkService::handleDatagram(const network::Datagram& datagram, Clock::time_point now)
{
	const auto& bytes = datagram.bytes;
	const network::ParsedHeader parsed = network::parseClientHeader(bytes);

	if (parsed.error) {
		_network.send(datagram.sender, network::makeError(*parsed.error, parsed));
		return;
	}
	if (!parsed.header) {
		return;
	}
	const network::Header& header = *parsed.header;
	if (header.type == network::PacketType::JoinRequest) {
		handleJoinDatagram(datagram, header, parsed, now);
	} else {
		handleSessionDatagram(datagram, header, parsed, now);
	}
}

/// Validates a join request before reading its nonce.
void ServerNetworkService::handleJoinDatagram(const network::Datagram& datagram, const network::Header& header,
	const network::ParsedHeader& parsed, Clock::time_point now)
{
	if (const auto error = network::validateClientPayload(header, datagram.bytes)) {
		_network.send(datagram.sender, network::makeError(*error, parsed));
		return;
	}
	handleJoin(datagram, network::readU64(datagram.bytes, network::HEADER_SIZE), now);
}

/// Validates and applies a packet for an existing session.
void ServerNetworkService::handleSessionDatagram(const network::Datagram& datagram, const network::Header& header,
	const network::ParsedHeader& parsed, Clock::time_point now)
{
	for (std::size_t index = 0; index < _sessions.size(); index++) {
		auto& session = _sessions[index];
		if (!session || session->id() != header.sessionId) {
			continue;
		}
		if (const auto error = network::validateClientPayload(header, datagram.bytes)) {
			_network.send(datagram.sender, network::makeError(*error, parsed));
			return;
		}
		session->refresh(datagram.sender, now);
		if (header.type == network::PacketType::Disconnect) {
			removeSession(static_cast<std::uint8_t>(index), 1);
		} else {
			session->acceptInput({header.sequence, datagram.bytes[network::HEADER_SIZE]}, now);
		}
		return;
	}
	_network.send(
		datagram.sender,
		network::makeError(network::ErrorCode::InvalidSession, parsed));
}

/// Creates a session or repeats its acceptance for a known nonce.
void ServerNetworkService::handleJoin(const network::Datagram& datagram, std::uint64_t nonce, Clock::time_point now)
{
	if (handleRepeatedJoin(datagram, nonce, now)) {
		return;
	}

	const auto freeIndex = findFreeSlot();
	if (!freeIndex) {
		sendJoinRejected(datagram, nonce);
		return;
	}
	const auto sessionId = createSessionId();
	if (!sessionId) {
		sendJoinRejected(datagram, nonce);
		return;
	}
	Session& session = _sessions[freeIndex.value()].emplace(datagram.sender, nonce, now, *sessionId);
	sendJoinAccepted(session, static_cast<std::uint8_t>(freeIndex.value()));
}

/// Refreshes a session and repeats its acceptance when the nonce is known.
bool ServerNetworkService::handleRepeatedJoin(const network::Datagram& datagram, std::uint64_t nonce, Clock::time_point now)
{
	for (std::size_t index = 0; index < _sessions.size(); index++) {
		auto& session = _sessions[index];
		if (!session || session->nonce() != nonce) {
			continue;
		}
		session->refresh(datagram.sender, now);
		sendJoinAccepted(*session, static_cast<std::uint8_t>(index));
		return true;
	}
	return false;
}

/// Finds a player slot available for a new session.
std::optional<std::size_t> ServerNetworkService::findFreeSlot() const
{
	for (std::size_t index = 0; index < _sessions.size(); index++) {
		if (!_sessions[index]) {
			return index;
		}
	}
	return std::nullopt;
}

/// Returns a nonzero ID that has never been assigned during this server run.
std::optional<std::uint32_t> ServerNetworkService::createSessionId()
{
	if (_issuedSessionIds.size() >= std::numeric_limits<std::uint32_t>::max()) {
		return std::nullopt;
	}
	for (std::size_t attempt = 0; attempt < 32; attempt++) {
		const std::uint32_t id = randomSessionId();
		if (id == 0) {
			continue;
		}
		const bool wasInserted = _issuedSessionIds.insert(id).second;
		if (wasInserted) {
			return id;
		}
	}
	return std::nullopt;
}

/// Sends an acceptance for a new or repeated join request.
void ServerNetworkService::sendJoinAccepted(const Session& session, std::uint8_t playerId)
{
	std::vector<std::uint8_t> payload;
	network::appendU64(payload, session.nonce());
	payload.push_back(playerId);
	_network.send(session.address(), network::makePacket(network::PacketType::JoinAccepted, 0, session.id(), payload));
}

/// Sends a server-full response for a join request.
void ServerNetworkService::sendJoinRejected(const network::Datagram& datagram, std::uint64_t nonce)
{
	std::vector<std::uint8_t> payload;
	network::appendU64(payload, nonce);
	payload.push_back(1);
	_network.send(datagram.sender, network::makePacket(network::PacketType::JoinRejected, 0, 0, payload));
}

/// Frees a player slot and announces the departure to the other sessions.
void ServerNetworkService::removeSession(std::uint8_t playerId, std::uint8_t reason)
{
	_sessions[playerId].reset();
	std::vector<std::uint8_t> payload;
	network::appendU64(payload, _tick);
	payload.push_back(playerId);
	payload.push_back(reason);
	broadcast(network::PacketType::PlayerLeft, payload);
}

/// Sends a session packet with the next nonzero outbound sequence.
void ServerNetworkService::sendTo(Session& session, network::PacketType type, const std::vector<std::uint8_t>& payload)
{
	const std::uint32_t sequence = session.takeNextSequence();
	_network.send(session.address(), network::makePacket(type, sequence, 0, payload));
}

/// Sends the same packet payload to every connected player.
void ServerNetworkService::broadcast(network::PacketType type, const std::vector<std::uint8_t>& payload)
{
	for (auto& session : _sessions) {
		if (session) {
			sendTo(*session, type, payload);
		}
	}
}

/// Sends one packet for each gameplay event, stamped with the tick that produced it.
void ServerNetworkService::broadcastEvents(const std::vector<WorldEvent>& events)
{
	for (const WorldEvent& event : events) {
		const GameWorld::EntityState& entity = event.entity();
		std::vector<std::uint8_t> payload;
		payload.reserve(24);
		network::appendU64(payload, _tick);
		network::PacketType type = network::PacketType::EntitySpawn;
		switch (event.type()) {
			case WorldEvent::Type::EntitySpawn:
				appendEntityRecord(payload, entity);
				break;
			case WorldEvent::Type::EntityDestroy:
				type = network::PacketType::EntityDestroy;
				network::appendU32(payload, entity.id);
				payload.push_back(static_cast<std::uint8_t>(entity.type));
				network::appendU32(payload, encodePosition(entity.x));
				network::appendU32(payload, encodePosition(entity.y));
				break;
			case WorldEvent::Type::EntityDamage:
				type = network::PacketType::EntityDamage;
				network::appendU32(payload, entity.id);
				payload.push_back(entity.health);
				break;
			case WorldEvent::Type::PlayerDeath:
				type = network::PacketType::PlayerDeath;
				payload.push_back(entity.owner);
				network::appendU32(payload, entity.id);
				network::appendU32(payload, encodePosition(entity.x));
				network::appendU32(payload, encodePosition(entity.y));
				break;
		}
		broadcast(type, payload);
	}
}

/// Expires silent sessions and returns fresh button states for this tick.
GameWorld::Input ServerNetworkService::collectInput(Clock::time_point now)
{
	GameWorld::Input input{{}, 0};
	for (std::size_t index = 0; index < _sessions.size(); index++) {
		auto& session = _sessions[index];
		if (!session) {
			continue;
		}
		if (session->hasExpired(now, SESSION_TIMEOUT)) {
			removeSession(static_cast<std::uint8_t>(index), 2);
			continue;
		}
		input.connectedMask |= static_cast<std::uint8_t>(1 << index);
		input.buttons[index] = session->buttonsAt(now, INPUT_TIMEOUT);
	}
	return input;
}

}
