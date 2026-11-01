/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface render system
*/

#include "engine/ui/UiRenderSystem.hpp"

#include <algorithm>
#include <vector>

#include "engine/ui/Components.hpp"

namespace engine::ui {

core::Signature UiRenderSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<UiTransform>());
	return signature;
}

/// Draws every interface element in screen space, in layer order, over the world.
void UiRenderSystem::render(core::Coordinator& coordinator, render::IGraphicsBackend& backend)
{
	backend.setRenderSpace(render::RenderSpace::Screen);
	const render::Point screen = backend.screenSize();
	std::vector<core::Entity> ordered(entities.begin(), entities.end());
	std::ranges::stable_sort(ordered, [&coordinator](core::Entity left, core::Entity right) {
		return coordinator.getComponent<UiTransform>(left).layer < coordinator.getComponent<UiTransform>(right).layer;
	});
	for (core::Entity entity : ordered) {
		const UiBounds bounds = resolveBounds(coordinator.getComponent<UiTransform>(entity), screen);
		if (coordinator.hasComponent<UiRect>(entity)) {
			backend.drawRect({.x = bounds.x, .y = bounds.y, .width = bounds.width, .height = bounds.height, .color = coordinator.getComponent<UiRect>(entity).color});
		}
		if (coordinator.hasComponent<UiImage>(entity)) {
			drawImage(coordinator.getComponent<UiImage>(entity), bounds, backend);
		}
		if (coordinator.hasComponent<UiText>(entity)) {
			drawText(coordinator.getComponent<UiText>(entity), bounds, backend);
		}
	}
}

/// Scales the image to fit the element without stretching it, and centers it.
void UiRenderSystem::drawImage(const UiImage& image, const UiBounds& bounds, render::IGraphicsBackend& backend)
{
	if (image.region.size.x <= 0 || image.region.size.y <= 0) {
		return;
	}
	const auto regionWidth = static_cast<float>(image.region.size.x);
	const auto regionHeight = static_cast<float>(image.region.size.y);
	const float scale = std::min(bounds.width / regionWidth, bounds.height / regionHeight);
	const float x = bounds.x + ((bounds.width - (regionWidth * scale)) / 2.0F);
	const float y = bounds.y + ((bounds.height - (regionHeight * scale)) / 2.0F);
	backend.drawSprite({.textureId = image.texture, .region = image.region, .x = x, .y = y, .scale = scale});
}

/// Aligns the text horizontally as asked and centers it vertically in the element.
void UiRenderSystem::drawText(const UiText& text, const UiBounds& bounds, render::IGraphicsBackend& backend)
{
	const render::Rect measured = backend.measureText(text.font, text.text, text.size);
	const auto left = static_cast<float>(measured.position.x);
	const auto top = static_cast<float>(measured.position.y);
	const auto width = static_cast<float>(measured.size.x);
	const auto height = static_cast<float>(measured.size.y);
	float x = bounds.x - left;
	if (text.align == TextAlign::Center) {
		x = bounds.x + ((bounds.width - width) / 2.0F) - left;
	}
	if (text.align == TextAlign::Right) {
		x = bounds.x + bounds.width - width - left;
	}
	const float y = bounds.y + ((bounds.height - height) / 2.0F) - top;
	backend.drawText({.font = text.font, .text = text.text, .x = x, .y = y, .size = text.size, .color = text.color});
}

}
