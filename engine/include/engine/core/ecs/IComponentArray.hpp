/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** IComponentArray
*/

#pragma once
#include "engine/core/ecs/Entity.hpp"
#include "engine/core/ecs/Types.hpp"

namespace engine::core {
class IComponentArray {
public:
	IComponentArray() = default;
	virtual ~IComponentArray() = default;
	virtual void entityDestroyed(Entity e) = 0;
};
}
