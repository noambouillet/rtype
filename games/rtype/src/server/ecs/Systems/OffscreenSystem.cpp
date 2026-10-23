/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** OffscreenSystem
*/

#include "OffscreenSystem.hpp"

#include <algorithm>

#include "engine/core/ecs/Components.hpp"
#include "server/ecs/GameComponents.hpp"

namespace rtype::ecs {

/// Builds the signature the system needs (Position + Offscreen).
engine::core::Signature OffscreenSystem::signature(engine::core::Coordinator& coordinator)
{
	engine::core::Signature sig;

	sig.set(coordinator.getComponentType<engine::core::Position>());
	sig.set(coordinator.getComponentType<Offscreen>());
	return sig;
}

/// Handles out-of-bounds entities by either clamping their position or destroying them.
void OffscreenSystem::update(engine::core::Coordinator& coordinator, float worldWidth, float worldHeight, float margin)
{
	for (const engine::core::Entity& e : entities) {
		auto& pos = coordinator.getComponent<engine::core::Position>(e);

		if (coordinator.getComponent<Offscreen>(e).action == OffscreenAction::Block) {
			engine::core::Collider box{};
			if (coordinator.hasComponent<engine::core::Collider>(e))
				box = coordinator.getComponent<engine::core::Collider>(e);
			pos.x = std::clamp(pos.x, -box.offsetX, worldWidth - box.offsetX - box.width);
			pos.y = std::clamp(pos.y, -box.offsetY, worldHeight - box.offsetY - box.height);
			continue;
		}
		const bool outside = pos.x < -margin || pos.x > worldWidth + margin || pos.y < -margin || pos.y > worldHeight + margin;

		if (outside && !coordinator.hasComponent<engine::core::Destroy>(e))
			coordinator.addComponent(e, engine::core::Destroy{});
	}
}

}
