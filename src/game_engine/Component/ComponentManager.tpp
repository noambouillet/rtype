/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentManager
*/

#pragma once
#include <utility>

#include "ComponentManager.hpp"

namespace rtype::ecs {
/// Register a new component type T and give it the next free ComponentType id.
template <typename T>
void ComponentManager::registerComponent()
{
	std::type_index key(typeid(T));
	assert(!_types.contains(key) && "component already exist");
	assert(_nextType < MAX_COMPONENTS && "too many components");

	ComponentType id = _nextType++;
	_types[key] = id;

	_arrays[id] = std::make_shared<ComponentArray<T>>(_maxEntities);
}

/// Get the ComponentType id associated with T.
template <typename T>
ComponentType ComponentManager::getType()
{
	auto id = _types.find(std::type_index(typeid(T)));
	assert(id != _types.end() && "component doesn't exist");

	return id->second;
}

/// Add a component of type T to the given entity.
template <typename T>
void ComponentManager::add(Entity e, T comp)
{
	getArray<T>()->insert(e, std::move(comp));
}

/// Remove the component of type T from the given entity.
template <typename T>
void ComponentManager::remove(Entity e)
{
	getArray<T>()->remove(e);
}

/// Check whether the given entity owns a component of type T.
template <typename T>
bool ComponentManager::has(Entity e)
{
	return getArray<T>()->has(e);
}

/// Get a reference to the component of type T of the given entity.
template <typename T>
T& ComponentManager::get(Entity e)
{
	return getArray<T>()->get(e);
}

/// Get the ComponentArray storing components of type T.
template <typename T>
std::shared_ptr<ComponentArray<T>> ComponentManager::getArray()
{
	ComponentType id = getType<T>();
	return std::static_pointer_cast<ComponentArray<T>>(_arrays[id]);
}
}
