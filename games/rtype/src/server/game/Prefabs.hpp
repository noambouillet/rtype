/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity prefabs: one function per kind of game entity
*/

#pragma once

#include <cstdint>

#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::prefabs {

struct EnemySpawn {
	float y;
	float firstShot;
};

bool canSpawnReplicated(engine::core::Coordinator& coordinator);
engine::core::Entity createPlayer(engine::core::Coordinator& coordinator, std::uint8_t playerId);
engine::core::Entity createPlayerProjectile(engine::core::Coordinator& coordinator, std::uint8_t playerId, engine::core::Position origin);
engine::core::Entity createEnemyProjectile(engine::core::Coordinator& coordinator, engine::core::Position origin, engine::core::Velocity velocity);
engine::core::Entity createPatapata(engine::core::Coordinator& coordinator, EnemySpawn spawn, float phase);
engine::core::Entity createBug(engine::core::Coordinator& coordinator, EnemySpawn spawn);
engine::core::Entity createScantBeam(engine::core::Coordinator& coordinator, engine::core::Position origin);
engine::core::Entity createBink(engine::core::Coordinator& coordinator, float firstShot);
engine::core::Entity createScant(engine::core::Coordinator& coordinator, EnemySpawn spawn);
engine::core::Entity createFormation(engine::core::Coordinator& coordinator, game::EnemyKind kind, float y, int count);
engine::core::Entity createWaveSpawner(engine::core::Coordinator& coordinator);

}
