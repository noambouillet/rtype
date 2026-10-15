/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Render system
*/

#include "RenderSystem.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "Components.hpp"

namespace rtype::ecs {

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

Signature RenderSystem::signature(Coordinator& coordinator)
{
	Signature signature;
	signature.set(coordinator.getComponentType<Position>());
	signature.set(coordinator.getComponentType<Sprite>());
	return signature;
}

/// Draws the scrolling background and every visual entity in layer order.
void RenderSystem::render(Coordinator& coordinator, render::IGraphicsBackend& backend, float deltaTime)
{
	_backgroundOffset += _backgroundSpeed * std::max(0.0F, deltaTime);
	backend.beginFrame();
	if (!_backgroundTexture.empty()) {
		backend.drawBackground(_backgroundTexture, _backgroundOffset);
	}
	std::vector<Entity> ordered(entities.begin(), entities.end());
	std::stable_sort(ordered.begin(), ordered.end(), [&coordinator](Entity left, Entity right) {
		return coordinator.getComponent<Sprite>(left).layer < coordinator.getComponent<Sprite>(right).layer;
	});
	for (Entity entity : ordered) {
		const Position& position = coordinator.getComponent<Position>(entity);
		const Sprite& sprite = coordinator.getComponent<Sprite>(entity);
		render::Rect region = sprite.region;
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
