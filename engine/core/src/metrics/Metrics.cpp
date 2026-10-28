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

/// Subscribes to every metrics event of `bus` and publishes a MetricsUpdated on it once per window.
/// `bus` must outlive this object; the subscriptions end when it is destroyed.
Metrics::Metrics(EventBus& bus)
	: _bus(bus), _snapshot{}, _window{}, _probes{}, _probeIndex(0), _roundTrip(std::nullopt), _smoothedRoundTrip(std::nullopt), _jitter(std::nullopt), _onFrame(bus.subscribe<FrameEnded>([this](const FrameEnded& event) { onFrame(event); })), _onBytesSent(bus.subscribe<BytesSent>([this](const BytesSent& event) {
		  _window.bytesSent += event.count;
		  _window.packetsSent++;
	  })),
	  _onBytesReceived(bus.subscribe<BytesReceived>([this](const BytesReceived& event) {
		  _window.bytesReceived += event.count;
		  _window.packetsReceived++;
	  })),
	  _onPingSent(bus.subscribe<PingSent>([this]([[maybe_unused]] const PingSent& event) { _window.pingsSent++; })),
	  _onRoundTrip(bus.subscribe<RoundTripMeasured>([this](const RoundTripMeasured& event) { onRoundTrip(event); }))
{
}

/// Returns the values of the last completed window, all zero before the first one.
const MetricsSnapshot& Metrics::snapshot() const
{
	return _snapshot;
}

/// Accumulates one frame and closes the window once it covers `WINDOW_SECONDS`.
void Metrics::onFrame(const FrameEnded& event)
{
	_window.elapsed += event.seconds;
	_window.busy += event.busySeconds;
	_window.worstFrame = std::max(_window.worstFrame, event.seconds);
	_window.worstBusy = std::max(_window.worstBusy, event.busySeconds);
	_window.frames++;
	if (_window.elapsed >= WINDOW_SECONDS) {
		closeWindow();
	}
}

/// Updates the latency estimates. The smoothing factors are the ones TCP uses (RFC 6298).
void Metrics::onRoundTrip(const RoundTripMeasured& event)
{
	_window.pingsAnswered++;
	if (!_roundTrip || !_smoothedRoundTrip || !_jitter) {
		_smoothedRoundTrip = event.milliseconds;
		_jitter = 0.0F;
	} else {
		const float variation = std::abs(event.milliseconds - *_roundTrip);
		*_jitter += (variation - *_jitter) * JITTER_SMOOTHING;
		*_smoothedRoundTrip += (event.milliseconds - *_smoothedRoundTrip) * ROUND_TRIP_SMOOTHING;
	}
	_roundTrip = event.milliseconds;
}

/// Turns the window totals into rates, publishes them and starts a new window.
void Metrics::closeWindow()
{
	const float seconds = _window.elapsed;
	const auto frames = static_cast<float>(_window.frames);
	_probes[_probeIndex] = {.sent = _window.pingsSent, .answered = _window.pingsAnswered};
	_probeIndex = (_probeIndex + 1) % LOSS_WINDOWS;
	_snapshot = {
		.framesPerSecond = frames / seconds,
		.averageFrameMs = seconds / frames * MS_PER_SECOND,
		.worstFrameMs = _window.worstFrame * MS_PER_SECOND,
		.averageBusyMs = _window.busy / frames * MS_PER_SECOND,
		.worstBusyMs = _window.worstBusy * MS_PER_SECOND,
		.uploadKiBps = static_cast<float>(_window.bytesSent) / BYTES_PER_KIB / seconds,
		.downloadKiBps = static_cast<float>(_window.bytesReceived) / BYTES_PER_KIB / seconds,
		.packetsSentPerSecond = static_cast<float>(_window.packetsSent) / seconds,
		.packetsReceivedPerSecond = static_cast<float>(_window.packetsReceived) / seconds,
		.roundTripMs = _roundTrip,
		.smoothedRoundTripMs = _smoothedRoundTrip,
		.jitterMs = _jitter,
		.packetLossPercent = packetLoss(),
	};
	_window = Window{};
	_bus.publish(MetricsUpdated{_snapshot});
}

/// Returns the share of probes left unanswered over the last `LOSS_WINDOWS` windows.
/// An answer arriving one window late still counts, so short bursts can read slightly low.
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
