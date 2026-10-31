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

#include "engine/core/Logger.hpp"

namespace engine::tools {

/// Writes a value with the given number of decimals, or "--" when it is not measured yet.
static std::string formatMetric(std::optional<float> value, int decimals)
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
MetricsOverlay::MetricsOverlay(core::EventBus& bus, render::FontId font)
	: _visible(false), _lines{"Collecting metrics..."}, _onUpdate(), _font(font)
{
	_onUpdate = bus.subscribe<core::MetricsUpdated>([this](const core::MetricsUpdated& event) { _lines = format(event.snapshot); });
	core::Logger::defaultLogger().debug("MetricsOverlay: Initialized metrics overlay");
}

void MetricsOverlay::toggle()
{
	_visible = !_visible;
	core::Logger::defaultLogger().debug("MetricsOverlay: Toggled visibility to " + std::string(_visible ? "visible" : "hidden"));
}

bool MetricsOverlay::visible() const
{
	return _visible;
}

void MetricsOverlay::setFont(render::FontId font)
{
	_font = font;
	core::Logger::defaultLogger().debug("MetricsOverlay: Updated font to ID " + std::to_string(font));
}

void MetricsOverlay::draw(render::IGraphicsBackend& backend) const
{
	draw(backend, _font);
}

/// Draws the panel in the top-left corner. Call it after drawing the scene, before ending the frame.
void MetricsOverlay::draw(render::IGraphicsBackend& backend, render::FontId font) const
{
	if (!_visible) {
		return;
	}
	const float height = (PADDING * 2.0F) + (LINE_HEIGHT * static_cast<float>(_lines.size()));
	backend.drawRect({.x = MARGIN, .y = MARGIN, .width = PANEL_WIDTH, .height = height, .color = PANEL_COLOR});
	float y = MARGIN + PADDING;
	for (const std::string& line : _lines) {
		backend.drawText({.font = font, .text = line, .x = MARGIN + PADDING, .y = y, .size = TEXT_SIZE, .color = TEXT_COLOR});
		y += LINE_HEIGHT;
	}
}

/// Turns a snapshot into the panel lines. Network lines read "--" until there is something to measure.
std::vector<std::string> MetricsOverlay::format(const core::MetricsSnapshot& snapshot)
{
	return {
		"FPS     " + formatMetric(snapshot.framesPerSecond, core::Metrics::COARSE_DECIMALS) + "   frame " + formatMetric(snapshot.averageFrameMs, core::Metrics::COARSE_DECIMALS) + " ms (worst " + formatMetric(snapshot.worstFrameMs, core::Metrics::COARSE_DECIMALS) + ") ",
		"Work    " + formatMetric(snapshot.averageBusyMs, core::Metrics::FINE_DECIMALS) + " ms (worst " + formatMetric(snapshot.worstBusyMs, core::Metrics::FINE_DECIMALS) + ") ",
		"Ping    " + formatMetric(snapshot.roundTripMs, core::Metrics::WHOLE_DECIMALS) + " ms   avg " + formatMetric(snapshot.smoothedRoundTripMs, core::Metrics::WHOLE_DECIMALS) + "   jitter " + formatMetric(snapshot.jitterMs, core::Metrics::COARSE_DECIMALS),
		"Loss    " + formatMetric(snapshot.packetLossPercent, core::Metrics::COARSE_DECIMALS) + " %",
		"Up      " + formatMetric(snapshot.uploadKiBps, core::Metrics::FINE_DECIMALS) + " KiB/s   " + formatMetric(snapshot.packetsSentPerSecond, core::Metrics::WHOLE_DECIMALS) + " pkt/s",
		"Down    " + formatMetric(snapshot.downloadKiBps, core::Metrics::FINE_DECIMALS) + " KiB/s   " + formatMetric(snapshot.packetsReceivedPerSecond, core::Metrics::WHOLE_DECIMALS) + " pkt/s",
	};
}

}
