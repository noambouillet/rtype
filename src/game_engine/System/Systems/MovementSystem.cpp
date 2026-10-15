/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MovementSystem
*/

#include "MovementSystem.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (Position + Velocity).
Signature MovementSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<Position>());
	sig.set(coordinator.getComponentType<Velocity>());
	return sig;
}

/// Applies each entity's velocity to its position for a frame of elapsedTime seconds.
void MovementSystem::update(Coordinator& coordinator, float elapsedTime)
{
	for (const Entity& e : entities) {
		auto& pos = coordinator.getComponent<Position>(e);
		const auto& vel = coordinator.getComponent<Velocity>(e);

		pos.x += vel.x * elapsedTime;
		pos.y += vel.y * elapsedTime;
	}
}
}
