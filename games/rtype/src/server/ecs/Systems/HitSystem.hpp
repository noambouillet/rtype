/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HitSystem
*/

#pragma once

#include <vector>

#include "CollisionSystem.hpp"
#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

class HitSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	std::vector<CollisionPair> update(Coordinator& coordinator, const std::vector<CollisionPair>& collisions);

private:
	void markHit(Coordinator& coordinator, Entity entity);
};

}
