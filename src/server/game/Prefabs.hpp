/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity prefabs: one function per kind of game entity
*/

#pragma once

#include <cstdint>

#include "Components.hpp"
#include "Coordinator.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::prefabs {

bool canSpawnReplicated(ecs::Coordinator& coordinator);
ecs::Entity createPlayer(ecs::Coordinator& coordinator, std::uint8_t playerId);
ecs::Entity createPlayerProjectile(ecs::Coordinator& coordinator, std::uint8_t playerId, ecs::Position origin);
ecs::Entity createEnemyProjectile(ecs::Coordinator& coordinator, ecs::Position origin, ecs::Velocity velocity);
ecs::Entity createPatapata(ecs::Coordinator& coordinator, float baseY, float phase, float firstShot);
ecs::Entity createBug(ecs::Coordinator& coordinator, float y, float firstShot);
ecs::Entity createScantBeam(ecs::Coordinator& coordinator, ecs::Position origin);
ecs::Entity createBink(ecs::Coordinator& coordinator, float firstShot);
ecs::Entity createScant(ecs::Coordinator& coordinator, float y, float firstShot);
ecs::Entity createFormation(ecs::Coordinator& coordinator, game::EnemyKind kind, float y, int count);
ecs::Entity createWaveSpawner(ecs::Coordinator& coordinator);

}
