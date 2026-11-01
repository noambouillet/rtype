/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface components, drawn in screen space
*/

#pragma once

#include <cstdint>
#include <string>

#include "engine/render/GraphicsBackend.hpp"

namespace engine::ui {

enum class Anchor : std::uint8_t {
	TopLeft,
	Top,
	TopRight,
	Left,
	Center,
	Right,
	BottomLeft,
	Bottom,
	BottomRight,
};

enum class TextAlign : std::uint8_t {
	Left,
	Center,
	Right,
};

struct UiTransform {
	Anchor anchor;
	float offsetX;
	float offsetY;
	float width;
	float height;
	int layer;
};

struct UiRect {
	render::Color color;
};

struct UiImage {
	render::TextureId texture;
	render::Rect region;
};

struct UiText {
	render::FontId font;
	std::string text;
	unsigned int size;
	render::Color color;
	TextAlign align;
};

struct UiButton {
	std::string action;
	bool hovered;
	bool pressed;
};

struct UiButtonStyle {
	render::Rect normal;
	render::Rect hovered;
	render::Rect pressed;
};

struct UiColorStyle {
	render::Color normal;
	render::Color hovered;
	render::Color pressed;
};

}
