/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HoverMotionSystem
*/

#pragma once

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::ecs {

class HoverMotionSystem : public engine::core::System {
public:
	static engine::core::Signature signature(engine::core::Coordinator& coordinator);
	void update(engine::core::Coordinator& coordinator, float elapsedTime);
};

}
