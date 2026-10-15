/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentArray
*/

#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "IComponentArray.hpp"

namespace rtype::ecs {
template <typename T>
class ComponentArray : public IComponentArray {
public:
	explicit ComponentArray(std::size_t maxEntities);
	~ComponentArray() override = default;
	bool has(Entity e) const;
	void insert(Entity e, T comp);
	void remove(Entity e);
	T& get(Entity e);
	void entityDestroyed(Entity e) override;

private:
	static constexpr std::size_t NONE = static_cast<std::size_t>(-1);
	std::vector<std::size_t> _sparse;
	std::vector<Entity> _dense;
	std::vector<T> _data;
};
}
#include "ComponentArray.tpp"
