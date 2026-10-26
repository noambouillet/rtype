/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Render system
*/

#include "engine/render/RenderSystem.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "engine/core/ecs/Components.hpp"
#include "engine/render/Components.hpp"

namespace engine::render {

RenderSystem::RenderSystem() : _backgroundTexture{}, _backgroundSpeed(0.0F), _backgroundOffset(0.0F)
{
}

/// Configures an optional scrolling background for this world.
void RenderSystem::setBackground(std::string texture, float speed)
{
	_backgroundTexture = std::move(texture);
	_backgroundSpeed = speed;
	_backgroundOffset = 0.0F;
}

core::Signature RenderSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<core::Position>());
	signature.set(coordinator.getComponentType<Sprite>());
	return signature;
}

/// Draws the scrolling background and every visual entity in layer order.
void RenderSystem::render(core::Coordinator& coordinator, IGraphicsBackend& backend, float deltaTime)
{
	_backgroundOffset += _backgroundSpeed * std::max(0.0F, deltaTime);
	backend.beginFrame();
	if (!_backgroundTexture.empty()) {
		backend.drawBackground(_backgroundTexture, _backgroundOffset);
	}
	std::vector<core::Entity> ordered(entities.begin(), entities.end());
	std::stable_sort(ordered.begin(), ordered.end(), [&coordinator](core::Entity left, core::Entity right) {
		return coordinator.getComponent<Sprite>(left).layer < coordinator.getComponent<Sprite>(right).layer;
	});
	for (core::Entity entity : ordered) {
		const core::Position& position = coordinator.getComponent<core::Position>(entity);
		const Sprite& sprite = coordinator.getComponent<Sprite>(entity);
		Rect region = sprite.region;
		if (coordinator.hasComponent<Animation>(entity)) {
			Animation& animation = coordinator.getComponent<Animation>(entity);
			if (!animation.frames.empty() && animation.frameSeconds > 0.0F) {
				const std::size_t frame = static_cast<std::size_t>(animation.elapsed / animation.frameSeconds);
				region = animation.frames[animation.loop ? frame % animation.frames.size() : std::min(frame, animation.frames.size() - 1)];
				animation.elapsed += std::max(0.0F, deltaTime);
			}
		}
		backend.drawSprite({sprite.textureName, region, position.x, position.y, sprite.originX, sprite.originY,
			sprite.scale, sprite.rotation});
	}
	backend.endFrame();
}

}
