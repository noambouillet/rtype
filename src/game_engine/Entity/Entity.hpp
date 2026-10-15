/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity
*/

#pragma once

#include <compare>
#include <cstdint>

namespace rtype::ecs {

class Entity {
public:
	using Id = std::uint32_t;
	using Index = std::uint32_t;
	using Generation = std::uint8_t;

	static constexpr Id INDEX_BITS = 24;
	static constexpr Id INDEX_MASK = (Id{1} << INDEX_BITS) - 1;
	static constexpr Id NULL_ID = 0xFFFFFFFF;

	Entity();
	explicit Entity(Index index, Generation generation);
	~Entity() = default;

	[[nodiscard]] Id id() const;
	[[nodiscard]] Index index() const;
	[[nodiscard]] Generation generation() const;
	[[nodiscard]] bool isNull() const;

	bool operator==(const Entity& other) const = default;
	auto operator<=>(const Entity& other) const = default;

private:
	Id _id;
};

}
