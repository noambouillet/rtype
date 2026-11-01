/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entities shared by the menu scenes
*/

#include "MenuWidgets.hpp"

#include <utility>

namespace rtype::client {

/// Creates a line of text centered horizontally, the offset of the spec below the middle of the screen.
engine::core::Entity MenuWidgets::makeLabel(engine::core::Coordinator& coordinator, engine::render::FontId font, LabelSpec spec)
{
	const engine::core::Entity label = coordinator.createEntity();
	const float height = static_cast<float>(spec.size) * LABEL_LINE_HEIGHT;
	coordinator.addComponent(label, engine::ui::UiTransform{.anchor = engine::ui::Anchor::Center, .offsetX = 0.0F, .offsetY = spec.offsetY, .width = LABEL_WIDTH, .height = height, .layer = BUTTON_LAYER});
	coordinator.addComponent(label, engine::ui::UiText{.font = font, .text = std::move(spec.text), .size = spec.size, .color = spec.color, .align = engine::ui::TextAlign::Center});
	return label;
}

/// Creates a button centered horizontally, the offset of the spec below the middle of the screen.
engine::core::Entity MenuWidgets::makeButton(engine::core::Coordinator& coordinator, engine::render::FontId font, ButtonSpec spec)
{
	const engine::core::Entity button = coordinator.createEntity();
	coordinator.addComponent(button, engine::ui::UiTransform{.anchor = engine::ui::Anchor::Center, .offsetX = 0.0F, .offsetY = spec.offsetY, .width = BUTTON_WIDTH, .height = BUTTON_HEIGHT, .layer = BUTTON_LAYER});
	coordinator.addComponent(button, engine::ui::UiRect{.color = BUTTON_NORMAL});
	coordinator.addComponent(button, engine::ui::UiColorStyle{.normal = BUTTON_NORMAL, .hovered = BUTTON_HOVERED, .pressed = BUTTON_PRESSED});
	coordinator.addComponent(button, engine::ui::UiText{.font = font, .text = std::move(spec.label), .size = BUTTON_TEXT_SIZE, .color = TEXT_COLOR, .align = engine::ui::TextAlign::Center});
	coordinator.addComponent(button, engine::ui::UiButton{.action = std::move(spec.action), .hovered = false, .pressed = false});
	coordinator.addComponent(button, engine::ui::UiFocusable{.order = spec.focusOrder});
	return button;
}

}
