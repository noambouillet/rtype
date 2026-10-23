/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** DestroySystem
*/

#pragma once

#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace engine::core {

class DestroySystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator);
};

}
