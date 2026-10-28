/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Authoritative server loop
*/

#include "GameServer.hpp"

#include <cstdint>
#include <thread>

#include "utils/network/Protocol.hpp"

namespace rtype {

GameServer::GameServer(engine::net::IServerTransport& transport) : _network(transport), _world{}
{
}

GameWorld::Input GameServer::tick()
{
	return _network.tick();
}

/// Advances the authoritative world at a fixed rate and publishes its results.
void GameServer::run(const volatile std::sig_atomic_t& running)
{
	auto nextTick = Clock::now();
	std::uint64_t ticksSinceSnapshot = 0;
	std::uint8_t previousConnectedMask = 0;
	const float deltaTime = std::chrono::duration<float>(TICK_DURATION).count();

	while (running != 0) {
		const GameWorld::Input input = tick();
		const std::vector<WorldEvent> events = _world.update(input, deltaTime);
		_network.broadcastEvents(events);
		ticksSinceSnapshot++;
		if (ticksSinceSnapshot >= network::SNAPSHOT_TICKS || input.connectedMask != previousConnectedMask) {
			_network.broadcastSnapshot(_world.replicatedEntities(), input.connectedMask, _world.aliveMask());
			ticksSinceSnapshot = 0;
		}
		previousConnectedMask = input.connectedMask;
		nextTick += TICK_DURATION;
		const auto now = Clock::now();
		if (nextTick < now) {
			nextTick = now + TICK_DURATION;
		}
		std::this_thread::sleep_until(nextTick);
	}
}

}
