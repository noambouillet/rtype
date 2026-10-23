/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HoverMotionSystem
*/

#pragma once

#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class HoverMotionSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator, float elapsedTime);
};

}
