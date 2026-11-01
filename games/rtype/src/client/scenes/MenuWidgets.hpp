/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entities shared by the menu scenes
*/

#pragma once

#include <string>

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/render/GraphicsBackend.hpp"
#include "engine/ui/Components.hpp"

namespace rtype::client {

struct LabelSpec {
	std::string text;
	unsigned int size;
	float offsetY;
	engine::render::Color color;
};

struct ButtonSpec {
	std::string label;
	std::string action;
	float offsetY;
	int focusOrder;
};

class MenuWidgets {
public:
	static constexpr float BUTTON_WIDTH = 320.0F;
	static constexpr float BUTTON_HEIGHT = 56.0F;
	static constexpr float BUTTON_SPACING = 72.0F;
	static constexpr unsigned int BUTTON_TEXT_SIZE = 26;
	static constexpr unsigned int TITLE_TEXT_SIZE = 64;
	static constexpr unsigned int MESSAGE_TEXT_SIZE = 20;
	static constexpr int BUTTON_LAYER = 10;
	static constexpr float LABEL_WIDTH = BUTTON_WIDTH * 3.0F;
	static constexpr float LABEL_LINE_HEIGHT = 1.5F;
	static constexpr engine::render::Color BUTTON_NORMAL{.r = 30, .g = 34, .b = 70, .a = 230};
	static constexpr engine::render::Color BUTTON_HOVERED{.r = 70, .g = 80, .b = 160, .a = 255};
	static constexpr engine::render::Color BUTTON_PRESSED{.r = 18, .g = 20, .b = 45, .a = 255};
	static constexpr engine::render::Color TEXT_COLOR{.r = 240, .g = 240, .b = 240, .a = 255};
	static constexpr engine::render::Color MESSAGE_COLOR{.r = 255, .g = 120, .b = 120, .a = 255};

	static engine::core::Entity makeLabel(engine::core::Coordinator& coordinator, engine::render::FontId font, LabelSpec spec);
	static engine::core::Entity makeButton(engine::core::Coordinator& coordinator, engine::render::FontId font, ButtonSpec spec);
};

}
