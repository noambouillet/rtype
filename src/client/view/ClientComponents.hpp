/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client view components
*/

#pragma once

#include <cstdint>
#include <optional>

#include "game/GameTypes.hpp"

namespace rtype::client {

struct NetworkIdentity {
	std::uint32_t serverId;
	game::EntityType kind;
	std::uint8_t owner;
	std::uint8_t variant;
};

enum class EffectKind : std::uint8_t {
	ShotBurst,
	HitSpark,
	MobExplosion,
	PlayerExplosion,
};

struct Effect {
	EffectKind kind;
	float remaining;
	std::optional<std::uint32_t> targetId;
	std::optional<std::uint8_t> followPlayer;
};

}
