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
static constexpr TextureId INVALID_TEXTURE_ID = 0;

struct Point {
	int x;
	int y;
};

struct Rect {
	Point position;
	Point size;
};

struct SpriteDraw {
	TextureId textureId = 0;
	Rect region{};
	float x = 0.0F;
	float y = 0.0F;
	float originX = 0.0F;
	float originY = 0.0F;
	float scale = 1.0F;
	float rotation = 0.0F;
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
