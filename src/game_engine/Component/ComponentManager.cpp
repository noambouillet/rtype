/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentManager
*/

#include "ComponentManager.hpp"

namespace rtype::ecs {
ComponentManager::ComponentManager(std::size_t maxEntities) : _maxEntities(maxEntities), _nextType(0)
{
}

/// Notify every component array that an entity was destroyed.
void ComponentManager::entityDestroyed(Entity e)
{
	for (auto& arr : _arrays) {
		if (arr) {
			arr->entityDestroyed(e);
		}
	}
}
}
