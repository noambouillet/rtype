/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Aggregates the metrics events published on an EventBus
*/

#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "engine/core/EventBus.hpp"
#include "engine/core/metrics/MetricsEvents.hpp"

namespace engine::core {

struct MetricsSnapshot {
	float framesPerSecond;
	float averageFrameMs;
	float worstFrameMs;
	float averageBusyMs;
	float worstBusyMs;
	float uploadKiBps;
	float downloadKiBps;
	float packetsSentPerSecond;
	float packetsReceivedPerSecond;
	std::optional<float> roundTripMs;
	std::optional<float> smoothedRoundTripMs;
	std::optional<float> jitterMs;
	std::optional<float> packetLossPercent;
};

struct MetricsUpdated {
	MetricsSnapshot snapshot;
};

class Metrics {
public:
	static constexpr float PERIOD_SECONDS = 1.0F;
	static constexpr int WHOLE_DECIMALS = 0;
	static constexpr int COARSE_DECIMALS = 1;
	static constexpr int FINE_DECIMALS = 2;

	explicit Metrics(EventBus& bus);

	Metrics(const Metrics&) = delete;
	Metrics& operator=(const Metrics&) = delete;

	[[nodiscard]] const MetricsSnapshot& snapshot() const;

private:
	static constexpr float MS_PER_SECOND = 1000.0F;
	static constexpr float BYTES_PER_KIB = 1024.0F;
	static constexpr float PERCENT = 100.0F;
	static constexpr float ROUND_TRIP_SMOOTHING = 0.125F;
	static constexpr float JITTER_SMOOTHING = 0.25F;
	static constexpr std::size_t LOSS_PERIODS = 5;

	struct PeriodTotals {
		float elapsed;
		float worstFrame;
		float busy;
		float worstBusy;
		std::size_t frames;
		std::size_t bytesSent;
		std::size_t bytesReceived;
		std::size_t packetsSent;
		std::size_t packetsReceived;
		std::size_t pingsSent;
		std::size_t pingsAnswered;
	};

	struct Latency {
		float roundTrip;
		float smoothedRoundTrip;
		float jitter;
	};

	struct ProbeCount {
		std::size_t sent;
		std::size_t answered;
	};

	void onFrame(const FrameEnded& event);
	void onRoundTrip(const RoundTripMeasured& event);
	void closePeriod();
	[[nodiscard]] std::optional<float> packetLoss() const;

	EventBus& _bus;
	MetricsSnapshot _snapshot;
	PeriodTotals _period;
	std::array<ProbeCount, LOSS_PERIODS> _probes;
	std::size_t _probeIndex;
	std::optional<Latency> _latency;
	Subscription _onFrame;
	Subscription _onBytesSent;
	Subscription _onBytesReceived;
	Subscription _onPingSent;
	Subscription _onRoundTrip;
};

}
