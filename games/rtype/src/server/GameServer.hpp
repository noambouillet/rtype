/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Authoritative server loop
*/

#pragma once

#include <chrono>
#include <csignal>

#include "engine/core/EventBus.hpp"
#include "engine/core/Logger.hpp"
#include "engine/core/metrics/Metrics.hpp"
#include "engine/net/Transport.hpp"
#include "game/GameWorld.hpp"
#include "network/ServerNetworkService.hpp"

namespace rtype {

class GameServer {
public:
	GameServer(engine::net::IServerTransport& transport, engine::core::Logger& logger);
	void run(const volatile std::sig_atomic_t& running);
	GameWorld::Input tick();

private:
	static constexpr auto TICK_DURATION = std::chrono::microseconds(16667);
	static constexpr auto SNAPSHOT_INTERVAL = std::chrono::milliseconds(50);
	static constexpr unsigned int METRICS_REPORT_WINDOWS = 10;
	static constexpr int WHOLE_DECIMALS = 0;
	static constexpr int COARSE_DECIMALS = 1;
	static constexpr int FINE_DECIMALS = 2;
	using Clock = std::chrono::steady_clock;

	void publishTick(Clock::time_point start, Clock::time_point& previousStart);
	void reportMetrics(const engine::core::MetricsSnapshot& snapshot);

	engine::core::Logger& _logger;
	engine::core::EventBus _bus;
	engine::core::Metrics _metrics;
	ServerNetworkService _network;
	GameWorld _world;
	unsigned int _windowsSinceReport;
	engine::core::Subscription _onMetrics;
};

}
