/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Graphics backend interface
*/

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace engine::render {

using TextureId = std::uint32_t;

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

	virtual TextureId loadTextureFromFile(const std::string& filepath) = 0;
	virtual void beginFrame() = 0;
	virtual void drawBackground(std::string_view texture, float offset) = 0;
	virtual void drawSprite(const SpriteDraw& sprite) = 0;
	virtual void endFrame() = 0;
};

}
