/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** System
*/

#pragma once

#include <set>

#include "engine/core/ecs/Entity.hpp"

namespace engine::core {

class System {
public:
	virtual ~System() = default;

	std::set<Entity> entities;
};

}
