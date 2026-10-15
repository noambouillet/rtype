/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentArray
*/

#pragma once
#include <utility>

#include "ComponentArray.hpp"

namespace rtype::ecs {
template <typename T>
ComponentArray<T>::ComponentArray(std::size_t maxEntities) : _sparse(maxEntities, NONE)
{
}

/// Check if a entity is present in the component array
template <typename T>
bool ComponentArray<T>::has(Entity e) const
{
	std::size_t index = e.index();
	return index < _sparse.size() && _sparse[index] != NONE && _dense[_sparse[index]] == e;
}

/// Insert a component for the given entity.
template <typename T>
void ComponentArray<T>::insert(Entity e, T comp)
{
	assert(e.index() < _sparse.size() && "entity index out of range");
	assert(!has(e) && "component already exist");
	assert(_sparse[e.index()] == NONE && "slot still used by a stale entity");
	_sparse[e.index()] = _dense.size();
	_dense.push_back(e);
	_data.push_back(std::move(comp));
}

/// Remove the component of the given entity, keeping the array packed.
template <typename T>
void ComponentArray<T>::remove(Entity e)
{
	assert(has(e) && "component doesn't exist");
	std::size_t removed = _sparse[e.index()];
	Entity last = _dense.back();

	_dense[removed] = last;
	_data[removed] = std::move(_data.back());
	_sparse[last.index()] = removed;

	_sparse[e.index()] = NONE;
	_dense.pop_back();
	_data.pop_back();
}

/// Get a reference to the component of the given entity.
template <typename T>
T& ComponentArray<T>::get(Entity e)
{
	assert(has(e) && "component doesn't exist");
	return _data[_sparse[e.index()]];
}

/// Remove the entity's component if it has one.
template <typename T>
void ComponentArray<T>::entityDestroyed(Entity e)
{
	if (has(e)) {
		remove(e);
	}
}
}
