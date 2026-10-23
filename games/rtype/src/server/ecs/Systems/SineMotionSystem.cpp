/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SineMotionSystem
*/

#include "SineMotionSystem.hpp"

#include <cmath>

#include "Components.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (SineMotion + Velocity): the enemies weaving on a sine wave.
Signature SineMotionSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<SineMotion>());
	sig.set(coordinator.getComponentType<Velocity>());
	return sig;
}

/// Sets each velocity to the derivative of motion while moving left.
void SineMotionSystem::update(Coordinator& coordinator, float elapsedTime)
{
	for (const Entity& e : entities) {
		auto& motion = coordinator.getComponent<SineMotion>(e);

		motion.elapsed += elapsedTime;
		coordinator.getComponent<Velocity>(e) = {-motion.speed,
			motion.amplitude * motion.frequency * std::cos(motion.frequency * motion.elapsed + motion.phase)};
	}
}

}
