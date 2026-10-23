/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DamageSystem
*/

#include "DamageSystem.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (Health): the entities that can be damaged.
engine::core::Signature DamageSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature sig;

	sig.set(coordinator.getComponentType<Health>());
	return sig;
}

/// Applies `Damage` in both directions for every pair in `collisions`.
std::vector<HealthChange> DamageSystem::update(engine::core::Coordinator& coordinator, const std::vector<engine::core::CollisionPair>& collisions)
{
	std::vector<HealthChange> changes;
	for (const auto& [a, b] : collisions) {
		if (const std::optional<HealthChange> change = applyDamage(coordinator, a, b)) {
			changes.push_back(*change);
		}
		if (const std::optional<HealthChange> change = applyDamage(coordinator, b, a)) {
			changes.push_back(*change);
		}
	}
	return changes;
}

/// Removes the `attacker`'s Damage from the `target`'s Health.
/// Marks the target with `Destroy` once its health reaches zero.
std::optional<HealthChange> DamageSystem::applyDamage(engine::core::Coordinator& coordinator, engine::core::Entity attacker, engine::core::Entity target)
{
	if (!entities.contains(target) || !coordinator.hasComponent<Damage>(attacker) || coordinator.hasComponent<engine::core::Destroy>(target)) {
		return std::nullopt;
	}

	auto& health = coordinator.getComponent<Health>(target);
	const auto& damage = coordinator.getComponent<Damage>(attacker);

	health.hp -= damage.amount;
	if (health.hp <= 0 && !coordinator.hasComponent<engine::core::Destroy>(target))
		coordinator.addComponent(target, engine::core::Destroy{});
	return HealthChange{target, health.hp};
}

}
