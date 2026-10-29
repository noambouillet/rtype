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
	explicit MetricsOverlay(core::EventBus& bus);

	MetricsOverlay(const MetricsOverlay&) = delete;
	MetricsOverlay& operator=(const MetricsOverlay&) = delete;

	void toggle();
	[[nodiscard]] bool visible() const;
	void draw(render::IGraphicsBackend& backend) const;

private:
	static constexpr float MARGIN = 8.0F;
	static constexpr float PADDING = 6.0F;
	static constexpr float LINE_HEIGHT = 18.0F;
	static constexpr float PANEL_WIDTH = 360.0F;
	static constexpr unsigned int TEXT_SIZE = 14;
	static constexpr render::Color PANEL_COLOR{0, 0, 0, 170};
	static constexpr render::Color TEXT_COLOR{235, 235, 235, 255};

	static std::vector<std::string> format(const core::MetricsSnapshot& snapshot);

	bool _visible;
	std::vector<std::string> _lines;
	core::Subscription _onUpdate;
};

}
