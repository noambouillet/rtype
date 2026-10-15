/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite sheet names and pixel regions
*/

#pragma once

#include <array>
#include <string_view>

#include "game_engine/Graphics/GraphicsBackend.hpp"

namespace rtype::sprites {

static constexpr std::string_view PLAYER_SHEET = "r-typesheet42.gif";
static constexpr std::array<render::Rect, 4> PLAYER_SHIPS = {
	render::Rect{{1, 69}, {32, 16}},  // Blue
	render::Rect{{1, 52}, {32, 16}},  // Red
	render::Rect{{1, 35}, {32, 16}},  // Green
	render::Rect{{1, 18}, {32, 16}},  // Magenta
};

static constexpr std::string_view ENEMY_SHEET = "r-typesheet5.gif";
static constexpr render::Rect ENEMY_PATAPATA = {{0, 0}, {33, 36}};

static constexpr std::string_view BUG_SHEET = "r-typesheet8.gif";
static constexpr render::Rect ENEMY_BUG = {{0, 0}, {33, 34}};

static constexpr std::string_view SHOT_SHEET = "r-typesheet1.gif";
static constexpr std::array<render::Rect, 4> PLAYER_SHOT_FRAMES = {
	render::Rect{{232, 103}, {16, 12}},
	render::Rect{{249, 103}, {16, 12}},
	render::Rect{{267, 103}, {16, 12}},
	render::Rect{{284, 103}, {16, 12}},
};
static constexpr std::array<render::Rect, 9> SHOT_BURST_FRAMES = {
	render::Rect{{199, 83}, {16, 16}},
	render::Rect{{215, 83}, {16, 16}},
	render::Rect{{231, 83}, {16, 16}},
	render::Rect{{247, 83}, {16, 16}},
	render::Rect{{263, 83}, {16, 16}},
	render::Rect{{279, 83}, {16, 16}},
	render::Rect{{295, 83}, {16, 16}},
	render::Rect{{311, 83}, {16, 16}},
	render::Rect{{327, 83}, {16, 16}},
};
static constexpr std::array<render::Rect, 7> HIT_SPARK_FRAMES = {
	render::Rect{{215, 83}, {16, 16}},
	render::Rect{{231, 83}, {16, 16}},
	render::Rect{{247, 83}, {16, 16}},
	render::Rect{{263, 83}, {16, 16}},
	render::Rect{{279, 83}, {16, 16}},
	render::Rect{{295, 83}, {16, 16}},
	render::Rect{{311, 83}, {16, 16}},
};

static constexpr std::string_view EXPLOSION_SHEET = "r-typesheet44.gif";
static constexpr std::array<int, 12> MOB_EXPLOSION_X = {129, 162, 195, 228, 261, 294, 327, 360, 393, 426, 459, 492};
static constexpr std::array<render::Rect, 12> MOB_EXPLOSION_FRAMES = [] {
	std::array<render::Rect, 12> frames{};
	for (std::size_t index = 0; index < frames.size(); index++) {
		frames[index] = render::Rect{{MOB_EXPLOSION_X[index], 0}, {33, 33}};
	}
	return frames;
}();
static constexpr std::array<int, 10> PLAYER_EXPLOSION_X = {1, 66, 131, 196, 261, 327, 392, 457, 522, 587};
static constexpr std::array<render::Rect, 10> PLAYER_EXPLOSION_FRAMES = [] {
	std::array<render::Rect, 10> frames{};
	for (std::size_t index = 0; index < frames.size(); index++) {
		frames[index] = render::Rect{{PLAYER_EXPLOSION_X[index], 35}, {64, 64}};
	}
	return frames;
}();

static constexpr std::string_view ENEMY_SHOT_SHEET = "r-typesheet43.gif";
static constexpr render::Rect ENEMY_SHOT = {{136, 6}, {7, 6}};

static constexpr std::string_view SCANT_SHEET = "r-typesheet9.gif";
static constexpr render::Rect ENEMY_SCANT = {{52, 3}, {56, 54}};
static constexpr render::Rect SCANT_BEAM = {{36, 59}, {54, 16}};

static constexpr std::string_view BINK_SHEET = "r-typesheet7.gif";
static constexpr render::Rect ENEMY_BINK = {{1, 4}, {32, 29}};

}
