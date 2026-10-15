/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Authoritative server loop
*/

#pragma once

#include <chrono>
#include <csignal>

#include "game/GameWorld.hpp"
#include "game_engine/Network/Transport.hpp"
#include "network/ServerNetworkService.hpp"

namespace rtype {

class GameServer {
public:
	explicit GameServer(network::IServerTransport& transport);
	void run(const volatile std::sig_atomic_t& running);
	GameWorld::Input tick();

private:
	static constexpr auto TICK_DURATION = std::chrono::microseconds(16667);
	static constexpr auto SNAPSHOT_INTERVAL = std::chrono::milliseconds(50);
	using Clock = std::chrono::steady_clock;

	ServerNetworkService _network;
	GameWorld _world;
};

}
