/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySpawnSystem
*/

#include "EnemySpawnSystem.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "server/ecs/GameComponents.hpp"
#include "server/game/GameConfig.hpp"
#include "server/game/Prefabs.hpp"

namespace rtype::ecs {

/// Seeds the random generator used for wave sizes and heights.
EnemySpawnSystem::EnemySpawnSystem() : _rng(std::random_device{}())
{
}

/// Builds the signature the system needs (WaveSpawner).
Signature EnemySpawnSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<WaveSpawner>());
	return sig;
}

/// Counts each spawner down and sends a wave when it reaches zero.
/// `active` is true when at least one player is connected
void EnemySpawnSystem::update(Coordinator& coordinator, float elapsedTime, bool active)
{
	if (!active)
		return;
	for (const Entity& e : entities) {
		auto& spawner = coordinator.getComponent<WaveSpawner>(e);

		spawner.timer -= elapsedTime;
		if (spawner.timer > 0.0F)
			continue;
		spawner.timer += spawner.interval;
		spawnWave(coordinator);
	}
}

/// Sends a lone Scant or a lone Bink on the ground now and then, otherwise a group of Patapata or a line of Bugs,
/// picked at random, at a random height kept far enough from the edges for their movement to stay inside the world.
void EnemySpawnSystem::spawnWave(Coordinator& coordinator)
{
	if (std::bernoulli_distribution(config::SCANT_WAVE_CHANCE)(_rng)) {
		std::uniform_real_distribution<float> height(config::SCANT_HEIGHT, config::WORLD_HEIGHT - config::SCANT_HEIGHT);

		prefabs::createFormation(coordinator, game::EnemyKind::Scant, height(_rng), 1);
		return;
	}
	if (std::bernoulli_distribution(config::BINK_WAVE_CHANCE)(_rng)) {
		prefabs::createFormation(coordinator, game::EnemyKind::Bink, config::GROUND_Y, 1);
		return;
	}
	if (std::bernoulli_distribution(0.5)(_rng)) {
		const float descent = -config::BUG_ARC_HEIGHT * std::sin(config::BUG_ARC_TURNS * std::numbers::pi_v<float>);
		std::uniform_int_distribution<int> size(config::BUG_MIN_COUNT, config::BUG_MAX_COUNT);
		std::uniform_real_distribution<float> height(config::BUG_ARC_HEIGHT + config::BUG_HEIGHT,
			config::WORLD_HEIGHT - std::max(descent, 0.0F) - config::BUG_HEIGHT);

		prefabs::createFormation(coordinator, game::EnemyKind::Bug, height(_rng), size(_rng));
		return;
	}
	const float margin = config::PATAPATA_SPREAD + config::PATAPATA_AMPLITUDE + config::PATAPATA_HEIGHT / 2.0F;
	std::uniform_int_distribution<int> size(config::PATAPATA_MIN_COUNT, config::PATAPATA_MAX_COUNT);
	std::uniform_real_distribution<float> height(margin, config::WORLD_HEIGHT - margin);

	prefabs::createFormation(coordinator, game::EnemyKind::Patapata, height(_rng), size(_rng));
}

}
