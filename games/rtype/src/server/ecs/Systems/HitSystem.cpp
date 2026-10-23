/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HitSystem
*/

#include "HitSystem.hpp"

#include "Components.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (DestroyOnHit).
Signature HitSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<DestroyOnHit>());
	return sig;
}

/// Keeps only the first collision for each entity destroyed on hit.
std::vector<CollisionPair> HitSystem::update(Coordinator& coordinator, const std::vector<CollisionPair>& collisions)
{
	std::vector<CollisionPair> accepted;
	accepted.reserve(collisions.size());
	for (const auto& [a, b] : collisions) {
		if ((entities.contains(a) && coordinator.hasComponent<Destroy>(a)) ||
			(entities.contains(b) && coordinator.hasComponent<Destroy>(b))) {
			continue;
		}
		accepted.push_back({a, b});
		markHit(coordinator, a);
		markHit(coordinator, b);
	}
	return accepted;
}

/// Adds `Destroy` to `entity` when this system tracks it and it is not already marked.
void HitSystem::markHit(Coordinator& coordinator, Entity entity)
{
	if (entities.contains(entity) && !coordinator.hasComponent<Destroy>(entity)) {
		coordinator.addComponent(entity, Destroy{});
	}
}

}
