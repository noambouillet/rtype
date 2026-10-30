/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** CollisionSystem
*/

#include "engine/physics/CollisionSystem.hpp"

#include <iterator>

namespace engine::physics {

/// Builds the signature the system needs (Position + Collider).
core::Signature CollisionSystem::signature(core::Coordinator& coordinator)
{
	core::Signature sig;

	sig.set(coordinator.getComponentType<core::Position>());
	sig.set(coordinator.getComponentType<Collider>());
	return sig;
}

/// Detects every pair of entities whose colliders overlap this frame.
void CollisionSystem::update(core::Coordinator& coordinator)
{
	_collisions.clear();
	for (auto a = entities.begin(); a != entities.end(); a++) {
		const auto& posA = coordinator.getComponent<core::Position>(*a);
		const auto& colA = coordinator.getComponent<Collider>(*a);

		for (auto b = std::next(a); b != entities.end(); b++) {
			const auto& posB = coordinator.getComponent<core::Position>(*b);
			const auto& colB = coordinator.getComponent<Collider>(*b);

			if (canCollide(colA, colB) && overlaps(posA, colA, posB, colB)) {
				_collisions.push_back({*a, *b});
			}
		}
	}
}

/// Returns the pairs detected by the last update.
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
bool CollisionSystem::overlaps(const core::Position& posA, const Collider& colA, const core::Position& posB, const Collider& colB)
{
	const float leftA = posA.x + colA.offsetX;
	const float topA = posA.y + colA.offsetY;
	const float leftB = posB.x + colB.offsetX;
	const float topB = posB.y + colB.offsetY;

	return leftA < leftB + colB.width && leftB < leftA + colA.width && topA < topB + colB.height && topB < topA + colA.height;
}

}
