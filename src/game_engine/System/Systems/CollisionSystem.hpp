/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CollisionSystem
*/

#pragma once

#include <vector>

#include "Components.hpp"
#include "Coordinator.hpp"
#include "System.hpp"

namespace rtype::ecs {

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
