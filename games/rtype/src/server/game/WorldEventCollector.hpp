/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Collects server gameplay events for one simulation tick
*/

#pragma once

#include <vector>

#include "engine/core/ecs/Coordinator.hpp"
#include "server/game/WorldEvent.hpp"

namespace rtype {

class WorldEventCollector {
public:
	void recordSpawn(engine::core::Coordinator& coordinator, engine::core::Entity entity);
	void recordDamage(engine::core::Coordinator& coordinator, engine::core::Entity entity);
	void recordRemoval(engine::core::Coordinator& coordinator, engine::core::Entity entity);
	std::vector<WorldEvent> take();

private:
	std::vector<WorldEvent> _events;
};

}
