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

namespace engine::core {

struct CollisionPair {
	Entity a;
	Entity b;
};

class CollisionSystem : public System {
public:
	Signature signature(Coordinator& coordinator);
	void update(Coordinator& coordinator);
	[[nodiscard]] const std::vector<CollisionPair>& collisions() const;

private:
	bool canCollide(const Collider& a, const Collider& b);
	bool overlaps(const Position& posA, const Collider& colA, const Position& posB, const Collider& colB);
	std::vector<CollisionPair> _collisions;
};

}
