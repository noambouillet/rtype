/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#pragma once

#include <cassert>

namespace rtype::ecs {

/// Registers a new system of type T. The SystemManager takes exclusive
/// ownership of the system and returns a reference for the user to manipulate.
template <typename T>
T& SystemManager::registerSystem()
{
	auto type = std::type_index(typeid(T));

	assert(!_systems.contains(type) && "System already registered.");
	_systems[type] = std::make_unique<T>();

	return static_cast<T&>(*_systems[type]);
}

/// Assigns a required component signature to a specific system type.
template <typename T>
void SystemManager::setSignature(Signature signature)
{
	auto type = std::type_index(typeid(T));

	assert(_systems.contains(type) &&
		   "System used before registered.");
	_signatures[type] = signature;
}

/// Returns a reference to an already registered system.
template <typename T>
T& SystemManager::getSystem()
{
	auto type = std::type_index(typeid(T));

	assert(_systems.contains(type) && "System not registered.");

	return static_cast<T&>(*_systems[type]);
}

}
