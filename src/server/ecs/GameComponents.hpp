/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** R-Type gameplay components (server side)
*/

#pragma once

#include <cstdint>

#include "game/GameTypes.hpp"

namespace rtype::ecs {

// type of entity to handle collision detection with tag/mask
namespace layer {
constexpr std::uint32_t PLAYER = 0b0001;
constexpr std::uint32_t ENEMY = 0b0010;
constexpr std::uint32_t PLAYER_PROJECTILE = 0b0100;
constexpr std::uint32_t ENEMY_PROJECTILE = 0b1000;
}

struct Replicated {
	game::EntityType type = game::EntityType::Enemy;
	std::uint8_t owner = game::NO_OWNER;
	std::uint8_t variant = 0;
};

struct PlayerControl {
	std::uint8_t playerId = 0;
	float speed = 0.0F;
};

struct Weapon {
	float cooldown = 0.0F;
	float remaining = 0.0F;
	bool firing = false;
	game::ShotKind shot = game::ShotKind::Aimed;
};

struct WaveSpawner {
	float interval = 0.0F;
	float timer = 0.0F;
};

enum class OffscreenAction : std::uint8_t {
	Destroy,
	Block,
};

struct Offscreen {
	OffscreenAction action = OffscreenAction::Destroy;
};

struct DestroyOnHit {};

struct SineMotion {
	float speed = 0.0F;
	float amplitude = 0.0F;
	float frequency = 0.0F;
	float phase = 0.0F;
	float elapsed = 0.0F;
};

struct FormationSpawner {
	game::EnemyKind kind = game::EnemyKind::Patapata;
	int remaining = 0;
	float delay = 0.0F;
	float timer = 0.0F;
	float y = 0.0F;
	float spread = 0.0F;
};

struct HoverMotion {
	float speed = 0.0F;
	float holdX = 0.0F;
	float trackSpeed = 0.0F;
	float stayTime = 0.0F;
	float chargeTime = 0.0F;
	float elapsed = 0.0F;
};

}
