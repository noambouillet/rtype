/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#include "SystemManager.hpp"

namespace rtype::ecs {

/// Removes the destroyed entity from all registered systems.
void SystemManager::entityDestroyed(Entity entity)
{
	for (auto const& [type, system] : _systems) {
		system->entities.erase(entity);
	}
}

/// Evaluates if an entity should be added to or removed from each system.
void SystemManager::entitySignatureChanged(Entity entity,
	Signature entitySignature)
{
	for (auto const& [type, system] : _systems) {
		auto const& systemSignature = _signatures[type];

		if ((entitySignature & systemSignature) == systemSignature) {
			system->entities.insert(entity);
		} else {
			system->entities.erase(entity);
		}
	}
}

}
