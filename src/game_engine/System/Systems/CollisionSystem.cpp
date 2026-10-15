/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CollisionSystem
*/

#include "CollisionSystem.hpp"

#include <iterator>

namespace rtype::ecs {

/// Builds the signature the system needs (Position + Collider).
Signature CollisionSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<Position>());
	sig.set(coordinator.getComponentType<Collider>());
	return sig;
}

/// Detects every pair of entities whose colliders overlap this frame.
void CollisionSystem::update(Coordinator& coordinator)
{
	_collisions.clear();
	for (auto a = entities.begin(); a != entities.end(); a++) {
		const auto& posA = coordinator.getComponent<Position>(*a);
		const auto& colA = coordinator.getComponent<Collider>(*a);

		for (auto b = std::next(a); b != entities.end(); b++) {
			const auto& posB = coordinator.getComponent<Position>(*b);
			const auto& colB = coordinator.getComponent<Collider>(*b);

			if (canCollide(colA, colB) && overlaps(posA, colA, posB, colB)) {
				_collisions.push_back({*a, *b});
			}
		}
	}
}

/// Returns the pairs detected by the last call to `update`.
const std::vector<CollisionPair>& CollisionSystem::collisions() const
{
	return _collisions;
}

/// Tells whether two colliders may interact: either mask contains the other's tag.
bool CollisionSystem::canCollide(const Collider& a, const Collider& b)
{
	return (a.mask & b.tag) != 0 || (b.mask & a.tag) != 0;
}

/// Tests whether two axis-aligned boxes overlap, offsets included.
bool CollisionSystem::overlaps(const Position& posA, const Collider& colA, const Position& posB, const Collider& colB)
{
	const float leftA = posA.x + colA.offsetX;
	const float topA = posA.y + colA.offsetY;
	const float leftB = posB.x + colB.offsetX;
	const float topB = posB.y + colB.offsetY;

	return leftA < leftB + colB.width && leftB < leftA + colA.width && topA < topB + colB.height && topB < topA + colA.height;
}

}
