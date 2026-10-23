/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Component
*/

#pragma once
#include <cassert>
#include <utility>

#include "Coordinator.hpp"

namespace rtype::ecs {
/// Register a new component type T so it can be added to entities.
template <typename T>
void Coordinator::registerComponent()
{
	_componentsManager.registerComponent<T>();
}

/// Add a component of type T to the entity, update its signature and notify the systems.
template <typename T>
void Coordinator::addComponent(Entity e, T comp)
{
	assert(_entitiesManager.isAlive(e) && "entity is not alive");
	_componentsManager.add<T>(e, std::move(comp));
	Signature sig = _entitiesManager.getSignature(e);
	sig.set(_componentsManager.getType<T>(), true);
	_entitiesManager.setSignature(e, sig);
	_systemsManager.entitySignatureChanged(e, sig);
}

/// Remove the component of type T from the entity, update its signature and notify the systems.
template <typename T>
void Coordinator::removeComponent(Entity e)
{
	assert(_entitiesManager.isAlive(e) && "entity is not alive");
	_componentsManager.remove<T>(e);
	Signature sig = _entitiesManager.getSignature(e);
	sig.set(_componentsManager.getType<T>(), false);
	_entitiesManager.setSignature(e, sig);
	_systemsManager.entitySignatureChanged(e, sig);
}

/// Check whether the given entity owns a component of type T.
template <typename T>
bool Coordinator::hasComponent(Entity e)
{
	return _componentsManager.has<T>(e);
}

/// Get a reference to the component of type T of the given entity.
template <typename T>
T& Coordinator::getComponent(Entity e)
{
	return _componentsManager.get<T>(e);
}

/// Get the ComponentType id of T, used to build system signatures.
template <typename T>
ComponentType Coordinator::getComponentType()
{
	return _componentsManager.getType<T>();
}
}
