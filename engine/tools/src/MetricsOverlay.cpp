/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** On-screen panel showing the latest metrics
*/

#include "engine/tools/MetricsOverlay.hpp"

#include <iomanip>
#include <optional>
#include <sstream>

namespace engine::tools {

/// Writes a value with the given number of decimals, or "--" when it is not measured yet.
static std::string number(std::optional<float> value, int decimals)
{
	if (!value) {
		return "--";
	}
	std::ostringstream text;
	text << std::fixed << std::setprecision(decimals) << *value;
	return text.str();
}

/// Shows the last MetricsUpdated published on a bus, which must outlive this object.
/// Starts hidden and keeps the text of the last period, so drawing never formats numbers.
MetricsOverlay::MetricsOverlay(core::EventBus& bus)
	: _visible(false), _lines{"Collecting metrics..."}, _onUpdate()
{
	_onUpdate = bus.subscribe<core::MetricsUpdated>([this](const core::MetricsUpdated& event) { _lines = format(event.snapshot); });
}

void MetricsOverlay::toggle()
{
	_visible = !_visible;
}

bool MetricsOverlay::visible() const
{
	return _visible;
}

/// Draws the panel in the top-left corner. Call it after drawing the scene, before ending the frame.
void MetricsOverlay::draw(render::IGraphicsBackend& backend) const
{
	if (!_visible) {
		return;
	}
	const float height = (PADDING * 2.0F) + (LINE_HEIGHT * static_cast<float>(_lines.size()));
	backend.drawRect({.x = MARGIN, .y = MARGIN, .width = PANEL_WIDTH, .height = height, .color = PANEL_COLOR});
	float y = MARGIN + PADDING;
	for (const std::string& line : _lines) {
		backend.drawText({.text = line, .x = MARGIN + PADDING, .y = y, .size = TEXT_SIZE, .color = TEXT_COLOR});
		y += LINE_HEIGHT;
	}
}

/// Turns a snapshot into the panel lines. Network lines read "--" until there is something to measure.
std::vector<std::string> MetricsOverlay::format(const core::MetricsSnapshot& snapshot)
{
	return {
		"FPS     " + number(snapshot.framesPerSecond, core::Metrics::COARSE_DECIMALS) + "   frame " + number(snapshot.averageFrameMs, core::Metrics::COARSE_DECIMALS) + " ms (worst " + number(snapshot.worstFrameMs, core::Metrics::COARSE_DECIMALS) + ")",
		"Work    " + number(snapshot.averageBusyMs, core::Metrics::FINE_DECIMALS) + " ms (worst " + number(snapshot.worstBusyMs, core::Metrics::FINE_DECIMALS) + ")",
		"Ping    " + number(snapshot.roundTripMs, core::Metrics::WHOLE_DECIMALS) + " ms   avg " + number(snapshot.smoothedRoundTripMs, core::Metrics::WHOLE_DECIMALS) + "   jitter " + number(snapshot.jitterMs, core::Metrics::COARSE_DECIMALS),
		"Loss    " + number(snapshot.packetLossPercent, core::Metrics::COARSE_DECIMALS) + " %",
		"Up      " + number(snapshot.uploadKiBps, core::Metrics::FINE_DECIMALS) + " KiB/s   " + number(snapshot.packetsSentPerSecond, core::Metrics::WHOLE_DECIMALS) + " pkt/s",
		"Down    " + number(snapshot.downloadKiBps, core::Metrics::FINE_DECIMALS) + " KiB/s   " + number(snapshot.packetsReceivedPerSecond, core::Metrics::WHOLE_DECIMALS) + " pkt/s",
	};
}

}
