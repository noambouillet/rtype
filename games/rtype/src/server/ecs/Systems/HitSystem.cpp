/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HitSystem
*/

#include "HitSystem.hpp"

#include "engine/core/ecs/Components.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (DestroyOnHit).
engine::core::Signature HitSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature sig;

	sig.set(coordinator.getComponentType<DestroyOnHit>());
	return sig;
}

/// Keeps only the first collision for each entity destroyed on hit.
std::vector<engine::physics::CollisionPair> HitSystem::update(engine::core::Coordinator& coordinator, const std::vector<engine::physics::CollisionPair>& collisions)
{
	std::vector<engine::physics::CollisionPair> accepted;
	accepted.reserve(collisions.size());
	for (const auto& [a, b] : collisions) {
		if ((entities.contains(a) && coordinator.hasComponent<engine::core::Destroy>(a)) ||
			(entities.contains(b) && coordinator.hasComponent<engine::core::Destroy>(b))) {
			continue;
		}
		accepted.push_back({a, b});
		markHit(coordinator, a);
		markHit(coordinator, b);
	}
	return accepted;
}

/// Marks an entity with Destroy when this system tracks it and it is not already marked.
void HitSystem::markHit(engine::core::Coordinator& coordinator, engine::core::Entity entity)
{
	if (entities.contains(entity) && !coordinator.hasComponent<engine::core::Destroy>(entity)) {
		coordinator.addComponent(entity, engine::core::Destroy{});
	}
}

}
