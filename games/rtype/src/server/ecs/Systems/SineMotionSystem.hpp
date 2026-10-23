/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SineMotionSystem
*/

#pragma once

#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class SineMotionSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator, float elapsedTime);
};

}
