/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** On-screen panel showing the latest metrics
*/

#pragma once

#include <string>
#include <vector>

#include "engine/core/EventBus.hpp"
#include "engine/core/metrics/Metrics.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace engine::tools {

class MetricsOverlay {
public:
	explicit MetricsOverlay(core::EventBus& bus, render::FontId font = render::INVALID_FONT_ID);

	MetricsOverlay(const MetricsOverlay&) = delete;
	MetricsOverlay& operator=(const MetricsOverlay&) = delete;

	void toggle();
	[[nodiscard]] bool visible() const;
	void setFont(render::FontId font);
	void draw(render::IGraphicsBackend& backend) const;
	void draw(render::IGraphicsBackend& backend, render::FontId font) const;

private:
	static constexpr float MARGIN = 8.0F;
	static constexpr float PADDING = 6.0F;
	static constexpr float LINE_HEIGHT = 18.0F;
	static constexpr float PANEL_WIDTH = 360.0F;
	static constexpr unsigned int TEXT_SIZE = 14;
	static constexpr render::Color PANEL_COLOR{.r = 0, .g = 0, .b = 0, .a = 170};
	static constexpr render::Color TEXT_COLOR{.r = 235, .g = 235, .b = 235, .a = 255};

	static std::vector<std::string> format(const core::MetricsSnapshot& snapshot);

	bool _visible;
	std::vector<std::string> _lines;
	core::Subscription _onUpdate;
	render::FontId _font;
};

}
