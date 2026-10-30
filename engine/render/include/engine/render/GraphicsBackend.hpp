/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Graphics backend interface
*/

#pragma once

#include <cstdint>
#include <string_view>

namespace engine::render {

struct Point {
	int x;
	int y;
};

struct Rect {
	Point position;
	Point size;
};

struct SpriteDraw {
	std::string_view texture;
	Rect region;
	float x;
	float y;
	float originX;
	float originY;
	float scale;
	float rotation;
};

struct Color {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
	std::uint8_t a;
};

struct RectDraw {
	float x;
	float y;
	float width;
	float height;
	Color color;
};

struct TextDraw {
	std::string_view text;
	float x;
	float y;
	unsigned int size;
	Color color;
};

class IGraphicsBackend {
public:
	virtual ~IGraphicsBackend() = default;
	virtual void beginFrame() = 0;
	virtual void drawBackground(std::string_view texture, float offset) = 0;
	virtual void drawSprite(const SpriteDraw& sprite) = 0;
	virtual void drawRect(const RectDraw& rect) = 0;
	virtual void drawText(const TextDraw& text) = 0;
	virtual void endFrame() = 0;
};

}
