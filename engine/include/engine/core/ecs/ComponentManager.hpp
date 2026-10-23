/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentManager
*/

#pragma once

#include <array>
#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "ComponentArray.hpp"

namespace rtype::ecs {
class ComponentManager {
public:
	explicit ComponentManager(std::size_t maxEntities);
	~ComponentManager() = default;
	template <typename T>
	void registerComponent();
	template <typename T>
	ComponentType getType();
	template <typename T>
	void add(Entity e, T comp);
	template <typename T>
	void remove(Entity e);
	template <typename T>
	bool has(Entity e);
	template <typename T>
	T& get(Entity e);
	void entityDestroyed(Entity e);

private:
	std::array<std::shared_ptr<IComponentArray>, MAX_COMPONENTS> _arrays;
	std::size_t _maxEntities;
	std::unordered_map<std::type_index, ComponentType> _types;
	ComponentType _nextType;
	template <typename T>
	std::shared_ptr<ComponentArray<T>> getArray();
};
}
#include "ComponentManager.tpp"
