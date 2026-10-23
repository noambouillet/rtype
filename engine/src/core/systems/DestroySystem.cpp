/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DestroySystem
*/

#include "DestroySystem.hpp"

#include <vector>

namespace rtype::ecs {

/// Builds the signature the system needs (Destroy): the entities marked for removal.
Signature DestroySystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<Destroy>());
	return sig;
}

/// Destroys every entity marked with `Destroy`.
void DestroySystem::update(Coordinator& coordinator)
{
	const std::vector<Entity> toDestroy(entities.begin(), entities.end());

	for (const Entity& e : toDestroy)
		coordinator.destroyEntity(e);
}

}
