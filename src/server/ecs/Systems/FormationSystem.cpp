/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** FormationSystem
*/

#include "FormationSystem.hpp"

#include <numbers>

#include "Components.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/game/GameConfig.hpp"
#include "server/game/Prefabs.hpp"

namespace rtype::ecs {

FormationSystem::FormationSystem() : _rng(std::random_device{}())
{
}

/// Returns how often an enemy of `kind` fires, which bounds the random delay before its first shot.
static float fireCooldown(game::EnemyKind kind)
{
	switch (kind) {
		case game::EnemyKind::Bug:
			return config::BUG_FIRE_COOLDOWN;
		case game::EnemyKind::Scant:
			return config::SCANT_FIRE_COOLDOWN;
		case game::EnemyKind::Bink:
			return config::BINK_FIRE_COOLDOWN;
		case game::EnemyKind::Patapata:
		default:
			return config::PATAPATA_FIRE_COOLDOWN;
	}
}

/// Builds the signature the system needs (FormationSpawner).
Signature FormationSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<FormationSpawner>());
	return sig;
}

/// Spawns one enemy of the formation's kind every `delay` seconds; the spawner is destroyed once all are out.
/// Bugs start at the same point and follow the path of the one before, Patapata get their own height within
/// `spread` and a random phase, Binks walk on the ground.
std::vector<Entity> FormationSystem::update(Coordinator& coordinator, float elapsedTime)
{
	std::vector<Entity> spawned;
	for (const Entity& e : entities) {
		auto& formation = coordinator.getComponent<FormationSpawner>(e);

		formation.timer -= elapsedTime;
		if (formation.timer > 0.0F || formation.remaining <= 0)
			continue;
		formation.timer += formation.delay;
		formation.remaining--;
		const bool done = formation.remaining <= 0;
		const game::EnemyKind kind = formation.kind;
		std::uniform_real_distribution<float> height(formation.y - formation.spread, formation.y + formation.spread);
		std::uniform_real_distribution<float> phase(0.0F, 2.0F * std::numbers::pi_v<float>);
		std::uniform_real_distribution<float> firstShot(config::ENEMY_FIRST_SHOT_MIN,
			config::ENEMY_FIRST_SHOT_MIN + fireCooldown(kind));
		const float y = height(_rng);
		if (prefabs::canSpawnReplicated(coordinator)) {
			if (kind == game::EnemyKind::Scant)
				spawned.push_back(prefabs::createScant(coordinator, y, firstShot(_rng)));
			else if (kind == game::EnemyKind::Bink)
				spawned.push_back(prefabs::createBink(coordinator, firstShot(_rng)));
			else if (kind == game::EnemyKind::Bug)
				spawned.push_back(prefabs::createBug(coordinator, y, firstShot(_rng)));
			else
				spawned.push_back(prefabs::createPatapata(coordinator, y, phase(_rng), firstShot(_rng)));
		}
		if (done && !coordinator.hasComponent<Destroy>(e))
			coordinator.addComponent(e, Destroy{});
	}
	return spawned;
}

}
