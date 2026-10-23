/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Collects server gameplay events for one simulation tick
*/

#include "WorldEventCollector.hpp"

#include <algorithm>
#include <optional>
#include <utility>

namespace rtype {

/// Records a completed replicated entity, including its initial position and owner.
void WorldEventCollector::recordSpawn(ecs::Coordinator& coordinator, ecs::Entity entity)
{
	const std::optional<ecs::ReplicatedState> state = coordinator.getSystem<ecs::ReplicationSystem>().capture(coordinator, entity);
	if (state) {
		_events.emplace_back(WorldEvent::Type::EntitySpawn, *state);
	}
}

/// Records the final surviving health of an entity once per tick.
void WorldEventCollector::recordDamage(ecs::Coordinator& coordinator, ecs::Entity entity)
{
	const std::optional<ecs::ReplicatedState> state = coordinator.getSystem<ecs::ReplicationSystem>().capture(coordinator, entity);
	if (!state || state->health == 0) {
		return;
	}
	for (WorldEvent& event : _events) {
		if (event.type() == WorldEvent::Type::EntityDamage && event.entity().id == state->id) {
			event = WorldEvent(WorldEvent::Type::EntityDamage, *state);
			return;
		}
	}
	_events.emplace_back(WorldEvent::Type::EntityDamage, *state);
}

/// Records a terminal event before the entity loses its components.
void WorldEventCollector::recordRemoval(ecs::Coordinator& coordinator, ecs::Entity entity)
{
	const std::optional<ecs::ReplicatedState> state = coordinator.getSystem<ecs::ReplicationSystem>().capture(coordinator, entity);
	if (!state) {
		return;
	}
	std::erase_if(_events, [id = state->id](const WorldEvent& event) {
		return event.type() == WorldEvent::Type::EntityDamage && event.entity().id == id;
	});
	const WorldEvent::Type type = state->type == game::EntityType::Player ? WorldEvent::Type::PlayerDeath : WorldEvent::Type::EntityDestroy;
	_events.emplace_back(type, *state);
}

/// Transfers the tick's events to the world owner.
std::vector<WorldEvent> WorldEventCollector::take()
{
	return std::move(_events);
}

}
