/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySpawnSystem
*/

#pragma once

#include <random>

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace rtype::ecs {

class EnemySpawnSystem : public engine::core::System {
public:
	EnemySpawnSystem();
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	void update(engine::core::Coordinator& coordinator, float elapsedTime, bool active);

private:
	void spawnWave(engine::core::Coordinator& coordinator);

	std::mt19937 _rng;
};

}
