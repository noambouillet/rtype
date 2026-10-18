/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Components
*/

#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "game_engine/Graphics/GraphicsBackend.hpp"

namespace rtype::ecs {

struct Position {
	float x = 0.0F;
	float y = 0.0F;
};

struct Velocity {
	float x = 0.0F;
	float y = 0.0F;
};

struct Health {
	int maxHP = 0;
	int hp = 0;
};

struct Damage {
	int amount = 0;
};

struct Sprite {
	std::string textureName;
	render::Rect region{};
	float originX = 0.0F;
	float originY = 0.0F;
	float scale = 1.0F;
	float rotation = 0.0F;
	int layer = 0;
};

struct Animation {
	std::vector<render::Rect> frames;
	float frameSeconds = 0.0F;
	float elapsed = 0.0F;
	bool loop = false;
};

struct Sound {
	std::string name;
	float volume = 100.0F;
};

struct Destroy {};

struct Collider {
	float width = 0.0F;
	float height = 0.0F;
	float offsetX = 0.0F;
	float offsetY = 0.0F;
	std::uint32_t tag = 0;
	std::uint32_t mask = 0;
};

}
