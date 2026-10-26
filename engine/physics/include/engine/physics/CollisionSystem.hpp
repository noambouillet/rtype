/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** CollisionSystem
*/

#pragma once

#include <vector>

#include "engine/core/ecs/Components.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/physics/Components.hpp"

namespace engine::physics {

struct CollisionPair {
	core::Entity a;
	core::Entity b;
};

class CollisionSystem : public core::System {
public:
	core::Signature signature(core::Coordinator& coordinator);
	void update(core::Coordinator& coordinator);
	[[nodiscard]] const std::vector<CollisionPair>& collisions() const;

private:
	bool canCollide(const Collider& a, const Collider& b);
	bool overlaps(const core::Position& posA, const Collider& colA, const core::Position& posB, const Collider& colB);
	std::vector<CollisionPair> _collisions;
};

}
