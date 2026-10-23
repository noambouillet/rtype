/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DamageSystem
*/

#pragma once

#include <optional>
#include <vector>

#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/core/systems/CollisionSystem.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

struct HealthChange {
	engine::core::Entity entity;
	int remainingHealth;
};

class DamageSystem : public engine::core::System {
public:
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::vector<HealthChange> update(engine::core::Coordinator& coordinator, const std::vector<engine::core::CollisionPair>& collisions);

private:
	std::optional<HealthChange> applyDamage(engine::core::Coordinator& coordinator, engine::core::Entity attacker, engine::core::Entity target);
};

}
