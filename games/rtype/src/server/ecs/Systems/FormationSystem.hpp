/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** FormationSystem
*/

#pragma once

#include <random>
#include <vector>

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::ecs {

class FormationSystem : public engine::core::System {
public:
	FormationSystem();
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::vector<engine::core::Entity> update(engine::core::Coordinator& coordinator, float elapsedTime);

private:
	std::mt19937 _rng;
};

}
