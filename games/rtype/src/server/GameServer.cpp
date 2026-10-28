/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Authoritative server loop
*/

#include "GameServer.hpp"

#include <iomanip>
#include <sstream>
#include <thread>

namespace rtype {

/// Creates the server. Its metrics are written to `logger` every `METRICS_REPORT_WINDOWS` windows while clients are talking to it.
GameServer::GameServer(engine::net::IServerTransport& transport, engine::core::Logger& logger)
	: _logger(logger), _bus(), _metrics(_bus), _network(transport, _bus), _world{}, _windowsSinceReport(0), _onMetrics(_bus.subscribe<engine::core::MetricsUpdated>([this](const engine::core::MetricsUpdated& event) { reportMetrics(event.snapshot); }))
{
}

GameWorld::Input GameServer::tick()
{
	return _network.tick();
}

/// Reports the tick that began at `start`: the time since the previous tick, and how long it worked before sleeping.
void GameServer::publishTick(Clock::time_point start, Clock::time_point& previousStart)
{
	const auto end = Clock::now();
	const float interval = std::chrono::duration<float>(start - previousStart).count();
	const float busy = std::chrono::duration<float>(end - start).count();
	previousStart = start;
	_bus.publish(engine::core::FrameEnded{.seconds = interval, .busySeconds = busy});
}

/// Logs one window out of `METRICS_REPORT_WINDOWS`, and none while no datagram arrives.
void GameServer::reportMetrics(const engine::core::MetricsSnapshot& snapshot)
{
	_windowsSinceReport++;
	if (_windowsSinceReport < METRICS_REPORT_WINDOWS || snapshot.packetsReceivedPerSecond == 0.0F) {
		return;
	}
	_windowsSinceReport = 0;
	const float budget = std::chrono::duration<float, std::milli>(TICK_DURATION).count();
	std::ostringstream text;
	text << std::fixed << std::setprecision(COARSE_DECIMALS) << "Metrics: " << snapshot.framesPerSecond << " ticks/s, tick work avg "
		 << std::setprecision(FINE_DECIMALS) << snapshot.averageBusyMs << " ms / worst " << snapshot.worstBusyMs << " ms (budget " << budget
		 << " ms), up " << snapshot.uploadKiBps << " KiB/s (" << std::setprecision(WHOLE_DECIMALS) << snapshot.packetsSentPerSecond
		 << " pkt/s), down " << std::setprecision(FINE_DECIMALS) << snapshot.downloadKiBps << " KiB/s (" << std::setprecision(WHOLE_DECIMALS)
		 << snapshot.packetsReceivedPerSecond << " pkt/s)";
	_logger.info(text.str());
}

/// Advances the authoritative world at a fixed rate and publishes its results.
void GameServer::run(const volatile std::sig_atomic_t& running)
{
	auto nextTick = Clock::now();
	auto snapshotElapsed = Clock::duration::zero();
	std::uint8_t previousConnectedMask = 0;
	const float deltaTime = std::chrono::duration<float>(TICK_DURATION).count();
	auto previousStart = nextTick - TICK_DURATION;

	while (running != 0) {
		const auto start = Clock::now();
		const GameWorld::Input input = tick();
		const std::vector<WorldEvent> events = _world.update(input, deltaTime);
		_network.broadcastEvents(events);
		snapshotElapsed += TICK_DURATION;
		if (snapshotElapsed >= SNAPSHOT_INTERVAL || input.connectedMask != previousConnectedMask) {
			_network.broadcastSnapshot(_world.replicatedEntities(), input.connectedMask, _world.aliveMask());
			snapshotElapsed %= SNAPSHOT_INTERVAL;
		}
		previousConnectedMask = input.connectedMask;
		publishTick(start, previousStart);
		nextTick += TICK_DURATION;
		const auto now = Clock::now();
		if (nextTick < now) {
			nextTick = now + TICK_DURATION;
		}
		std::this_thread::sleep_until(nextTick);
	}
}

}
