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

#include "engine/core/ecs/Components.hpp"
#include "server/ecs/GameComponents.hpp"
#include "server/ecs/Systems/PlayerControlSystem.hpp"

namespace rtype::ecs {

/// Returns the height of the player closest to `origin`, or nothing when no player is in the game.
static std::optional<float> closestPlayerY(engine::core::Coordinator& coordinator, engine::core::Position origin)
{
	std::optional<float> y;
	float closest = 0.0F;

	for (const engine::core::Entity& player : coordinator.getSystem<PlayerControlSystem>().entities) {
		const auto& target = coordinator.getComponent<engine::core::Position>(player);
		const float distance = std::hypot(target.x - origin.x, target.y - origin.y);

		if (y && distance >= closest) {
			continue;
		}
		closest = distance;
		y = target.y;
	}
	return y;
}

/// Builds the signature the system needs (HoverMotion + Position + Velocity): the enemies hovering in front of the players.
engine::core::Signature HoverMotionSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature sig;

	sig.set(coordinator.getComponentType<HoverMotion>());
	sig.set(coordinator.getComponentType<engine::core::Position>());
	sig.set(coordinator.getComponentType<engine::core::Velocity>());
	return sig;
}

/// Flies each hoverer in to `holdX`, follows the closest player's height there, then flies it off once `stayTime` is up.
void HoverMotionSystem::update(engine::core::Coordinator& coordinator, float elapsedTime)
{
	for (const engine::core::Entity& e : entities) {
		auto& motion = coordinator.getComponent<HoverMotion>(e);
		const engine::core::Position position = coordinator.getComponent<engine::core::Position>(e);
		engine::core::Velocity velocity{.x = 0.0F, .y = 0.0F};

		motion.elapsed += elapsedTime;
		if (motion.elapsed >= motion.stayTime) {
			velocity.x = -motion.speed;
		} else if (position.x > motion.holdX) {
			velocity.x = -std::min(motion.speed, (position.x - motion.holdX) / elapsedTime);
		} else {
			const bool charging = coordinator.hasComponent<Weapon>(e) && coordinator.getComponent<Weapon>(e).firing && coordinator.getComponent<Weapon>(e).remaining <= motion.chargeTime;
			const std::optional<float> targetY = closestPlayerY(coordinator, position);

			if (targetY && !charging) {
				velocity.y = std::clamp((*targetY - position.y) / elapsedTime, -motion.trackSpeed, motion.trackSpeed);
			}
		}
		coordinator.getComponent<engine::core::Velocity>(e) = velocity;
	}
}

}
