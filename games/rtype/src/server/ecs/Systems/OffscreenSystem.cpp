/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** OffscreenSystem
*/

#include "OffscreenSystem.hpp"

#include <algorithm>

#include "Components.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (Position + Offscreen).
Signature OffscreenSystem::signature(Coordinator& coordinator)
{
	Signature sig;

	sig.set(coordinator.getComponentType<Position>());
	sig.set(coordinator.getComponentType<Offscreen>());
	return sig;
}

/// Handles out-of-bounds entities by either clamping their position or destroying them.
void OffscreenSystem::update(Coordinator& coordinator, float worldWidth, float worldHeight, float margin)
{
	for (const Entity& e : entities) {
		auto& pos = coordinator.getComponent<Position>(e);

		if (coordinator.getComponent<Offscreen>(e).action == OffscreenAction::Block) {
			Collider box{};
			if (coordinator.hasComponent<Collider>(e))
				box = coordinator.getComponent<Collider>(e);
			pos.x = std::clamp(pos.x, -box.offsetX, worldWidth - box.offsetX - box.width);
			pos.y = std::clamp(pos.y, -box.offsetY, worldHeight - box.offsetY - box.height);
			continue;
		}
		const bool outside = pos.x < -margin || pos.x > worldWidth + margin || pos.y < -margin || pos.y > worldHeight + margin;

		if (outside && !coordinator.hasComponent<Destroy>(e))
			coordinator.addComponent(e, Destroy{});
	}
}

}
