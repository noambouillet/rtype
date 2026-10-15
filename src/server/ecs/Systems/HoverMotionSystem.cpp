/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** HoverMotionSystem
*/

#include "HoverMotionSystem.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

#include "Components.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/ecs/Systems/PlayerControlSystem.hpp"

namespace rtype::ecs {

/// Returns the height of the player closest to `origin`, or nothing when no player is in the game.
static std::optional<float> closestPlayerY(Coordinator& coordinator, Position origin)
{
	std::optional<float> y;
	float closest = 0.0F;

	for (const Entity& player : coordinator.getSystem<PlayerControlSystem>().entities) {
		const auto& target = coordinator.getComponent<Position>(player);
		const float distance = std::hypot(target.x - origin.x, target.y - origin.y);

		if (y && distance >= closest)
			continue;
		closest = distance;
		y = target.y;
	}
	return y;
}

/// Builds the signature the system needs (HoverMotion + Position + Velocity): the enemies hovering in front of the players.
Signature HoverMotionSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<HoverMotion>());
	sig.set(coordinator.getComponentType<Position>());
	sig.set(coordinator.getComponentType<Velocity>());
	return sig;
}

/// Flies each hoverer in to `holdX`, follows the closest player's height there, then flies it off once `stayTime` is up.
void HoverMotionSystem::update(Coordinator& coordinator, float elapsedTime)
{
	for (const Entity& e : entities) {
		auto& motion = coordinator.getComponent<HoverMotion>(e);
		const Position position = coordinator.getComponent<Position>(e);
		Velocity velocity{0.0F, 0.0F};

		motion.elapsed += elapsedTime;
		if (motion.elapsed >= motion.stayTime) {
			velocity.x = -motion.speed;
		} else if (position.x > motion.holdX) {
			velocity.x = -std::min(motion.speed, (position.x - motion.holdX) / elapsedTime);
		} else {
			const bool charging = coordinator.hasComponent<Weapon>(e) && coordinator.getComponent<Weapon>(e).firing && coordinator.getComponent<Weapon>(e).remaining <= motion.chargeTime;
			const std::optional<float> targetY = closestPlayerY(coordinator, position);

			if (targetY && !charging)
				velocity.y = std::clamp((*targetY - position.y) / elapsedTime, -motion.trackSpeed, motion.trackSpeed);
		}
		coordinator.getComponent<Velocity>(e) = velocity;
	}
}

}
