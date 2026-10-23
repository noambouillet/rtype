/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Coordinator
*/

#include "engine/core/ecs/Coordinator.hpp"

#include <stdexcept>

#include "engine/core/Error.hpp"

namespace engine::core {
Coordinator::Coordinator(std::size_t maxEntities)
	: _entitiesManager(maxEntities), _componentsManager(_entitiesManager.maxEntities())
{
}

/// Creates a new entity. Throws if the maximum number of entities is reached.
Entity Coordinator::createEntity()
{
	std::optional<Entity> newEntity = _entitiesManager.create();

	if (!newEntity)
		throw CoordinatorError("max entities reached");
	return *newEntity;
}

/// Destroy an entity and remove it from every component array and system.
/// Does nothing if the handle is stale or already dead.
void Coordinator::destroyEntity(Entity e)
{
	if (!_entitiesManager.destroy(e))
		return;
	_componentsManager.entityDestroyed(e);
	_systemsManager.entityDestroyed(e);
}

/// Tell whether the given entity handle still refers to a living entity.
bool Coordinator::isAlive(Entity e) const
{
	return _entitiesManager.isAlive(e);
}
}
