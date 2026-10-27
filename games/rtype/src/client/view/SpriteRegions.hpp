/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite sheet names and pixel regions
*/

#pragma once

#include <array>
#include <string_view>

#include "engine/render/GraphicsBackend.hpp"

namespace rtype::sprites {

static constexpr std::string_view PLAYER_SHEET = "r-typesheet42.gif";
static constexpr std::array<engine::render::Rect, 4> PLAYER_SHIPS = {
	engine::render::Rect{.position = {.x = 1, .y = 69}, .size = {.x = 32, .y = 16}},  // Blue
	engine::render::Rect{.position = {.x = 1, .y = 52}, .size = {.x = 32, .y = 16}},  // Red
	engine::render::Rect{.position = {.x = 1, .y = 35}, .size = {.x = 32, .y = 16}},  // Green
	engine::render::Rect{.position = {.x = 1, .y = 18}, .size = {.x = 32, .y = 16}},  // Magenta
};

static constexpr std::string_view ENEMY_SHEET = "r-typesheet5.gif";
static constexpr engine::render::Rect ENEMY_PATAPATA = {.position = {.x = 0, .y = 0}, .size = {.x = 33, .y = 36}};

static constexpr std::string_view BUG_SHEET = "r-typesheet8.gif";
static constexpr engine::render::Rect ENEMY_BUG = {.position = {.x = 0, .y = 0}, .size = {.x = 33, .y = 34}};

static constexpr std::string_view SHOT_SHEET = "r-typesheet1.gif";
static constexpr std::array<engine::render::Rect, 4> PLAYER_SHOT_FRAMES = {
	engine::render::Rect{.position = {.x = 232, .y = 103}, .size = {.x = 16, .y = 12}},
	engine::render::Rect{.position = {.x = 249, .y = 103}, .size = {.x = 16, .y = 12}},
	engine::render::Rect{.position = {.x = 267, .y = 103}, .size = {.x = 16, .y = 12}},
	engine::render::Rect{.position = {.x = 284, .y = 103}, .size = {.x = 16, .y = 12}},
};
static constexpr std::array<engine::render::Rect, 9> SHOT_BURST_FRAMES = {
	engine::render::Rect{.position = {.x = 199, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 215, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 231, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 247, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 263, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 279, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 295, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 311, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 327, .y = 83}, .size = {.x = 16, .y = 16}},
};
static constexpr std::array<engine::render::Rect, 7> HIT_SPARK_FRAMES = {
	engine::render::Rect{.position = {.x = 215, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 231, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 247, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 263, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 279, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 295, .y = 83}, .size = {.x = 16, .y = 16}},
	engine::render::Rect{.position = {.x = 311, .y = 83}, .size = {.x = 16, .y = 16}},
};

static constexpr std::string_view EXPLOSION_SHEET = "r-typesheet44.gif";
static constexpr std::array<int, 12> MOB_EXPLOSION_X = {129, 162, 195, 228, 261, 294, 327, 360, 393, 426, 459, 492};
static constexpr std::array<engine::render::Rect, 12> MOB_EXPLOSION_FRAMES = [] {
	std::array<engine::render::Rect, 12> frames{};
	for (std::size_t index = 0; index < frames.size(); index++) {
		frames[index] = engine::render::Rect{.position = {.x = MOB_EXPLOSION_X[index], .y = 0}, .size = {.x = 33, .y = 33}};
	}
	return frames;
}();
static constexpr std::array<int, 10> PLAYER_EXPLOSION_X = {1, 66, 131, 196, 261, 327, 392, 457, 522, 587};
static constexpr std::array<engine::render::Rect, 10> PLAYER_EXPLOSION_FRAMES = [] {
	std::array<engine::render::Rect, 10> frames{};
	for (std::size_t index = 0; index < frames.size(); index++) {
		frames[index] = engine::render::Rect{.position = {.x = PLAYER_EXPLOSION_X[index], .y = 35}, .size = {.x = 64, .y = 64}};
	}
	return frames;
}();

static constexpr std::string_view ENEMY_SHOT_SHEET = "r-typesheet43.gif";
static constexpr engine::render::Rect ENEMY_SHOT = {.position = {.x = 136, .y = 6}, .size = {.x = 7, .y = 6}};

static constexpr std::string_view SCANT_SHEET = "r-typesheet9.gif";
static constexpr engine::render::Rect ENEMY_SCANT = {.position = {.x = 52, .y = 3}, .size = {.x = 56, .y = 54}};
static constexpr engine::render::Rect SCANT_BEAM = {.position = {.x = 36, .y = 59}, .size = {.x = 54, .y = 16}};

static constexpr std::string_view BINK_SHEET = "r-typesheet7.gif";
static constexpr engine::render::Rect ENEMY_BINK = {.position = {.x = 1, .y = 4}, .size = {.x = 32, .y = 29}};
}
