/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Gameplay event produced by one server world tick
*/

#pragma once

#include <cstdint>

#include "server/ecs/Systems/ReplicationSystem.hpp"

namespace rtype {

class WorldEvent {
public:
	enum class Type : std::uint8_t { EntitySpawn,
		EntityDestroy,
		EntityDamage,
		PlayerDeath };

	WorldEvent(Type type, ecs::ReplicatedState entity);
	[[nodiscard]] Type type() const;
	[[nodiscard]] const ecs::ReplicatedState& entity() const;

private:
	Type _type;
	ecs::ReplicatedState _entity;
};

}
