/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DamageSystem
*/

#pragma once

#include <optional>
#include <vector>

#include "CollisionSystem.hpp"
#include "Components.hpp"
#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

struct HealthChange {
	Entity entity;
	int remainingHealth;
};

class DamageSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	std::vector<HealthChange> update(Coordinator& coordinator, const std::vector<CollisionPair>& collisions);

private:
	std::optional<HealthChange> applyDamage(Coordinator& coordinator, Entity attacker, Entity target);
};

}
