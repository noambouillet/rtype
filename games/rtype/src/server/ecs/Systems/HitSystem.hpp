/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HitSystem
*/

#pragma once

#include <vector>

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/physics/CollisionSystem.hpp"

namespace rtype::ecs {

class HitSystem : public engine::core::System {
public:
	engine::core::Signature signature(engine::core::Coordinator& coordinator);
	std::vector<engine::physics::CollisionPair> update(engine::core::Coordinator& coordinator, const std::vector<engine::physics::CollisionPair>& collisions);

private:
	void markHit(engine::core::Coordinator& coordinator, engine::core::Entity entity);
};

}
