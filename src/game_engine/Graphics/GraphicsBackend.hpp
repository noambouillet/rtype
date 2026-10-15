/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Graphics backend interface
*/

#pragma once

#include <string_view>

namespace rtype::render {

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

class IGraphicsBackend {
public:
	virtual ~IGraphicsBackend() = default;
	virtual void beginFrame() = 0;
	virtual void drawBackground(std::string_view texture, float offset) = 0;
	virtual void drawSprite(const SpriteDraw& sprite) = 0;
	virtual void endFrame() = 0;
};

}
