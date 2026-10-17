/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Client presentation events
*/

#pragma once

#include <cstdint>
#include <variant>

#include "client/GameConfig.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

struct SpawnEvent {
	std::uint32_t id;
	game::EntityType kind;
	std::uint8_t owner;
	float x;
	float y;
};

struct DestroyEvent {
	std::uint32_t id;
	game::EntityType kind;
	float x;
	float y;
};

struct DamageEvent {
	std::uint32_t id;
};

struct DeathEvent {
	std::uint32_t id;
	float x;
	float y;
};

using ViewEvent = std::variant<SpawnEvent, DestroyEvent, DamageEvent, DeathEvent>;

/// Tells a kill inside the world apart from an entity that left it.
inline bool destroyedOnscreen(const DestroyEvent& event)
{
	constexpr float margin = client_config::OFFSCREEN_MARGIN;
	return event.x >= -margin && event.x <= static_cast<float>(client_config::WINDOW_WIDTH) + margin &&
		   event.y >= -margin && event.y <= static_cast<float>(client_config::WINDOW_HEIGHT) + margin;
}

}
