/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ReplicationSystem
*/

#include "ReplicationSystem.hpp"

#include <algorithm>

#include "engine/core/ecs/Components.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/game/GameConfig.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (Position + Replicated): the entities clients can see.
engine::core::Signature ReplicationSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature sig;

	sig.set(coordinator.getComponentType<engine::core::Position>());
	sig.set(coordinator.getComponentType<Replicated>());
	return sig;
}

/// Captures one replicated entity while its components still exist.
std::optional<ReplicatedState> ReplicationSystem::capture(engine::core::Coordinator& coordinator, engine::core::Entity entity) const
{
	if (!entities.contains(entity) || !coordinator.isAlive(entity)) {
		return std::nullopt;
	}
	const Replicated& info = coordinator.getComponent<Replicated>(entity);
	const engine::core::Position& position = coordinator.getComponent<engine::core::Position>(entity);
	int health = 0;
	if (coordinator.hasComponent<Health>(entity)) {
		health = std::clamp(coordinator.getComponent<Health>(entity).hp, 0, 255);
	}
	return ReplicatedState{.id = entity.id(), .type = info.type, .owner = info.owner, .variant = info.variant, .health = static_cast<std::uint8_t>(health), .x = position.x, .y = position.y};
}

/// Lists what clients must see of every replicated entity (at most 64).
std::vector<ReplicatedState> ReplicationSystem::collect(engine::core::Coordinator& coordinator)
{
	std::vector<ReplicatedState> states;
	states.reserve(std::min(entities.size(), config::MAX_REPLICATED_ENTITIES));
	for (const engine::core::Entity& e : entities) {
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
