/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Authoritative server loop
*/

#include "GameServer.hpp"

#include <thread>

namespace rtype {

GameServer::GameServer(network::IServerTransport& transport) : _network(transport), _world{}
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
	auto snapshotElapsed = Clock::duration::zero();
	std::uint8_t previousConnectedMask = 0;
	const float deltaTime = std::chrono::duration<float>(TICK_DURATION).count();

	while (running != 0) {
		const GameWorld::Input input = tick();
		const std::vector<WorldEvent> events = _world.update(input, deltaTime);
		_network.broadcastEvents(events);
		snapshotElapsed += TICK_DURATION;
		if (snapshotElapsed >= SNAPSHOT_INTERVAL || input.connectedMask != previousConnectedMask) {
			_network.broadcastSnapshot(_world.replicatedEntities(), input.connectedMask, _world.aliveMask());
			snapshotElapsed %= SNAPSHOT_INTERVAL;
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
