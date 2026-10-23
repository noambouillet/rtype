/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Collects server gameplay events for one simulation tick
*/

#pragma once

#include <vector>

#include "Coordinator.hpp"
#include "server/game/WorldEvent.hpp"

namespace rtype {

class WorldEventCollector {
public:
	void recordSpawn(ecs::Coordinator& coordinator, ecs::Entity entity);
	void recordDamage(ecs::Coordinator& coordinator, ecs::Entity entity);
	void recordRemoval(ecs::Coordinator& coordinator, ecs::Entity entity);
	std::vector<WorldEvent> take();

private:
	std::vector<WorldEvent> _events;
};

}
