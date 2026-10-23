/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ReplicationSystem
*/

#include "ReplicationSystem.hpp"

#include <algorithm>

#include "Components.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/game/GameConfig.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (Position + Replicated): the entities clients can see.
Signature ReplicationSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<Position>());
	sig.set(coordinator.getComponentType<Replicated>());
	return sig;
}

/// Captures one replicated entity while its components still exist.
std::optional<ReplicatedState> ReplicationSystem::capture(Coordinator& coordinator, Entity entity) const
{
	if (!entities.contains(entity) || !coordinator.isAlive(entity)) {
		return std::nullopt;
	}
	const Replicated& info = coordinator.getComponent<Replicated>(entity);
	const Position& position = coordinator.getComponent<Position>(entity);
	int health = 0;
	if (coordinator.hasComponent<Health>(entity)) {
		health = std::clamp(coordinator.getComponent<Health>(entity).hp, 0, 255);
	}
	return ReplicatedState{entity.id(), info.type, info.owner, info.variant, static_cast<std::uint8_t>(health), position.x, position.y};
}

/// Lists what clients must see of every replicated entity (at most 64).
std::vector<ReplicatedState> ReplicationSystem::collect(Coordinator& coordinator)
{
	std::vector<ReplicatedState> states;
	states.reserve(std::min(entities.size(), config::MAX_REPLICATED_ENTITIES));
	for (const Entity& e : entities) {
		if (states.size() >= config::MAX_REPLICATED_ENTITIES) {
			break;
		}
		if (const std::optional<ReplicatedState> state = capture(coordinator, e)) {
			states.push_back(*state);
		}
	}
	return states;
}

}
