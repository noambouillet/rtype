/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MovementSystem
*/

#pragma once

#include "Components.hpp"
#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class MovementSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator, float elapsedTime);
};

}
