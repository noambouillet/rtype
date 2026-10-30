/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Aggregates the metrics events published on an EventBus
*/

#include "engine/core/metrics/Metrics.hpp"

#include <algorithm>
#include <cmath>

namespace engine::core {

/// Subscribes to every metrics event of a bus and publishes a MetricsUpdated on it once per period.
/// The bus must outlive this object.
Metrics::Metrics(EventBus& bus)
	: _bus(bus), _snapshot{}, _period{}, _probes{}, _probeIndex(0), _latency(std::nullopt), _onFrame(), _onBytesSent(), _onBytesReceived(), _onPingSent(), _onRoundTrip()
{
	_onFrame = bus.subscribe<FrameEnded>([this](const FrameEnded& event) { onFrame(event); });
	_onBytesSent = bus.subscribe<BytesSent>([this](const BytesSent& event) {
		_period.bytesSent += event.count;
		_period.packetsSent++;
	});
	_onBytesReceived = bus.subscribe<BytesReceived>([this](const BytesReceived& event) {
		_period.bytesReceived += event.count;
		_period.packetsReceived++;
	});
	_onPingSent = bus.subscribe<PingSent>([this]([[maybe_unused]] const PingSent& event) { _period.pingsSent++; });
	_onRoundTrip = bus.subscribe<RoundTripMeasured>([this](const RoundTripMeasured& event) { onRoundTrip(event); });
}

/// Returns the values of the last completed period, all zero before the first one.
const MetricsSnapshot& Metrics::snapshot() const
{
	return _snapshot;
}

/// Accumulates one frame and closes the period once it lasts PERIOD_SECONDS.
void Metrics::onFrame(const FrameEnded& event)
{
	_period.elapsed += event.seconds;
	_period.busy += event.busySeconds;
	_period.worstFrame = std::max(_period.worstFrame, event.seconds);
	_period.worstBusy = std::max(_period.worstBusy, event.busySeconds);
	_period.frames++;
	if (_period.elapsed >= PERIOD_SECONDS) {
		closePeriod();
	}
}

/// Updates the latency estimates. The smoothing factors are the ones TCP uses (RFC 6298).
void Metrics::onRoundTrip(const RoundTripMeasured& event)
{
	_period.pingsAnswered++;
	if (!_latency) {
		_latency = Latency{.roundTrip = event.milliseconds, .smoothedRoundTrip = event.milliseconds, .jitter = 0.0F};
		return;
	}
	const float variation = std::abs(event.milliseconds - _latency->roundTrip);
	_latency->jitter += (variation - _latency->jitter) * JITTER_SMOOTHING;
	_latency->smoothedRoundTrip += (event.milliseconds - _latency->smoothedRoundTrip) * ROUND_TRIP_SMOOTHING;
	_latency->roundTrip = event.milliseconds;
}

/// Turns the period totals into rates, publishes them and starts a new period.
void Metrics::closePeriod()
{
	const float seconds = _period.elapsed;
	const auto frames = static_cast<float>(_period.frames);
	_probes[_probeIndex] = {.sent = _period.pingsSent, .answered = _period.pingsAnswered};
	_probeIndex = (_probeIndex + 1) % LOSS_PERIODS;
	_snapshot = {
		.framesPerSecond = frames / seconds,
		.averageFrameMs = seconds / frames * MS_PER_SECOND,
		.worstFrameMs = _period.worstFrame * MS_PER_SECOND,
		.averageBusyMs = _period.busy / frames * MS_PER_SECOND,
		.worstBusyMs = _period.worstBusy * MS_PER_SECOND,
		.uploadKiBps = static_cast<float>(_period.bytesSent) / BYTES_PER_KIB / seconds,
		.downloadKiBps = static_cast<float>(_period.bytesReceived) / BYTES_PER_KIB / seconds,
		.packetsSentPerSecond = static_cast<float>(_period.packetsSent) / seconds,
		.packetsReceivedPerSecond = static_cast<float>(_period.packetsReceived) / seconds,
		.roundTripMs = _latency ? std::optional(_latency->roundTrip) : std::nullopt,
		.smoothedRoundTripMs = _latency ? std::optional(_latency->smoothedRoundTrip) : std::nullopt,
		.jitterMs = _latency ? std::optional(_latency->jitter) : std::nullopt,
		.packetLossPercent = packetLoss(),
	};
	_period = PeriodTotals{};
	_bus.publish(MetricsUpdated{_snapshot});
}

/// Returns the share of probes left unanswered over the last LOSS_PERIODS periods.
/// An answer arriving one period late still counts, so short bursts can read slightly low.
std::optional<float> Metrics::packetLoss() const
{
	std::size_t sent = 0;
	std::size_t answered = 0;
	for (const ProbeCount& count : _probes) {
		sent += count.sent;
		answered += count.answered;
	}
	if (sent == 0) {
		return std::nullopt;
	}
	const std::size_t lost = sent > answered ? sent - answered : 0;
	return static_cast<float>(lost) / static_cast<float>(sent) * PERCENT;
}

}
