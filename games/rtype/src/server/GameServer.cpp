/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Authoritative server loop
*/

#include "GameServer.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <thread>

#include "utils/network/Protocol.hpp"

namespace rtype {

/// Creates the server. Its metrics are logged every METRICS_REPORT_PERIODS periods while clients are talking to it.
GameServer::GameServer(engine::net::IServerTransport& transport, engine::core::Logger& logger)
	: _logger(logger), _bus(), _metrics(_bus), _network(transport, _bus), _world{}, _periodsSinceReport(0), _onMetrics()
{
	_onMetrics = _bus.subscribe<engine::core::MetricsUpdated>([this](const engine::core::MetricsUpdated& event) { reportMetrics(event.snapshot); });
}

GameWorld::Input GameServer::tick()
{
	return _network.tick();
}

/// Reports a tick: the time since the previous one, and how long it worked before sleeping.
void GameServer::publishTick(Clock::time_point start, Clock::time_point& previousStart)
{
	const auto end = Clock::now();
	const float interval = std::chrono::duration<float>(start - previousStart).count();
	const float busy = std::chrono::duration<float>(end - start).count();
	previousStart = start;
	_bus.publish(engine::core::FrameEnded{.seconds = interval, .busySeconds = busy});
}

/// Logs one period out of METRICS_REPORT_PERIODS, and none while no datagram arrives.
void GameServer::reportMetrics(const engine::core::MetricsSnapshot& snapshot)
{
	_periodsSinceReport++;
	if (_periodsSinceReport < METRICS_REPORT_PERIODS || snapshot.packetsReceivedPerSecond == 0.0F) {
		return;
	}
	_periodsSinceReport = 0;
	const float budget = std::chrono::duration<float, std::milli>(TICK_DURATION).count();
	std::ostringstream text;
	text << std::fixed << std::setprecision(engine::core::Metrics::COARSE_DECIMALS) << "Metrics: " << snapshot.framesPerSecond << " ticks/s, tick work avg "
		 << std::setprecision(engine::core::Metrics::FINE_DECIMALS) << snapshot.averageBusyMs << " ms / worst " << snapshot.worstBusyMs << " ms (budget " << budget
		 << " ms), up " << snapshot.uploadKiBps << " KiB/s (" << std::setprecision(engine::core::Metrics::WHOLE_DECIMALS) << snapshot.packetsSentPerSecond
		 << " pkt/s), down " << std::setprecision(engine::core::Metrics::FINE_DECIMALS) << snapshot.downloadKiBps << " KiB/s (" << std::setprecision(engine::core::Metrics::WHOLE_DECIMALS)
		 << snapshot.packetsReceivedPerSecond << " pkt/s)";
	_logger.debug(text.str());
}

/// Advances the authoritative world at a fixed rate and publishes its results.
void GameServer::run(const volatile std::sig_atomic_t& running)
{
	auto nextTick = Clock::now();
	std::uint64_t ticksSinceSnapshot = 0;
	std::uint8_t previousConnectedMask = 0;
	const float deltaTime = std::chrono::duration<float>(TICK_DURATION).count();
	auto previousStart = nextTick - TICK_DURATION;

	while (running != 0) {
		const auto start = Clock::now();
		const GameWorld::Input input = tick();
		const std::vector<WorldEvent> events = _world.update(input, deltaTime);
		_network.broadcastEvents(events);
		ticksSinceSnapshot++;
		if (ticksSinceSnapshot >= network::SNAPSHOT_TICKS || input.connectedMask != previousConnectedMask) {
			_network.broadcastSnapshot(_world.replicatedEntities(), input.connectedMask, _world.aliveMask());
			ticksSinceSnapshot = 0;
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
