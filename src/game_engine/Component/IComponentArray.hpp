/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** IComponentArray
*/

#pragma once
#include "Entity.hpp"
#include "Types.hpp"

namespace rtype::ecs {
class IComponentArray {
public:
	IComponentArray() = default;
	virtual ~IComponentArray() = default;
	virtual void entityDestroyed(Entity e) = 0;
};
}  // namepsace rtype::ecs
