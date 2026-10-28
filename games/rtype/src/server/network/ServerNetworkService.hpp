/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Server network service
*/

#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

#include "engine/net/Transport.hpp"
#include "server/Session.hpp"
#include "server/game/GameWorld.hpp"
#include "utils/network/Protocol.hpp"

namespace rtype {

class ServerNetworkService {
public:
	explicit ServerNetworkService(engine::net::IServerTransport& transport);
	GameWorld::Input tick();
	void broadcastEvents(const std::vector<WorldEvent>& events);
	void broadcastSnapshot(const std::vector<GameWorld::EntityState>& entities,
		std::uint8_t connectedMask, std::uint8_t aliveMask);

private:
	static constexpr auto INPUT_TIMEOUT = std::chrono::milliseconds(250);
	static constexpr auto SESSION_TIMEOUT = std::chrono::seconds(5);
	static constexpr std::size_t SNAPSHOT_HEADER_SIZE = 11;
	static constexpr std::size_t ENTITY_RECORD_SIZE = 16;
	using Clock = std::chrono::steady_clock;

	void handleDatagram(const engine::net::Datagram& datagram, Clock::time_point now);
	void handleJoinDatagram(const engine::net::Datagram& datagram, const network::Header& header,
		const network::ParsedHeader& parsed, Clock::time_point now);
	void handleSessionDatagram(const engine::net::Datagram& datagram, const network::Header& header,
		const network::ParsedHeader& parsed, Clock::time_point now);
	void handleJoin(const engine::net::Datagram& datagram, std::uint64_t nonce, Clock::time_point now);
	bool handleRepeatedJoin(const engine::net::Datagram& datagram, std::uint64_t nonce, Clock::time_point now);
	std::optional<std::size_t> findFreeSlot() const;
	std::optional<std::uint32_t> createSessionId();
	void sendJoinAccepted(const Session& session, std::uint8_t playerId);
	void sendJoinRejected(const engine::net::Datagram& datagram, std::uint64_t nonce);
	void removeSession(std::uint8_t playerId, std::uint8_t reason);
	void sendTo(Session& session, network::PacketType type, const std::vector<std::uint8_t>& payload);
	void broadcast(network::PacketType type, const std::vector<std::uint8_t>& payload);
	GameWorld::Input collectInput(Clock::time_point now);
	std::vector<std::uint8_t> buildSnapshot(const std::vector<GameWorld::EntityState>& entities,
		std::uint8_t connectedMask, std::uint8_t aliveMask) const;

	engine::net::IServerTransport& _network;
	std::array<std::optional<Session>, GameWorld::MAX_PLAYERS> _sessions;
	std::unordered_set<std::uint32_t> _issuedSessionIds;
	std::uint64_t _tick;
};

}
