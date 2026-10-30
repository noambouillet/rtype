/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Render components
*/

#pragma once

#include <vector>

#include "engine/render/GraphicsBackend.hpp"

namespace engine::render {

struct Sprite {
	TextureId texture = INVALID_TEXTURE_ID;
	Rect region{};
	float originX = 0.0F;
	float originY = 0.0F;
	float scale = 1.0F;
	float rotation = 0.0F;
	int layer = 0;
};

struct Animation {
	TextureId texture = INVALID_TEXTURE_ID;
	std::vector<Rect> frames;
	float frameSeconds = 0.0F;
	float elapsed = 0.0F;
	bool loop = false;
};

}
