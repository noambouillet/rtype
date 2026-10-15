/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Shared game types
*/

#pragma once

#include <cstdint>

namespace rtype::game {

static constexpr std::uint8_t NO_OWNER = 255;
static constexpr std::uint8_t BUTTON_UP = 0x01;
static constexpr std::uint8_t BUTTON_DOWN = 0x02;
static constexpr std::uint8_t BUTTON_LEFT = 0x04;
static constexpr std::uint8_t BUTTON_RIGHT = 0x08;
static constexpr std::uint8_t BUTTON_FIRE = 0x10;
static constexpr std::uint8_t MAX_BUTTON_STATE = 0b00011111;

enum class EntityType : std::uint8_t {
	Player = 1,
	Enemy = 2,
	PlayerProjectile = 3,
	EnemyProjectile = 4,
};

enum class EnemyKind : std::uint8_t {
	Patapata = 0,
	Bug = 1,
	Scant = 2,
	Bink = 3,
};

enum class ShotKind : std::uint8_t {
	Aimed = 0,
	Beam = 1,
};

}
