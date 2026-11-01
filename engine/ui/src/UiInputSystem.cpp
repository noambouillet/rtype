/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface input system
*/

#include "engine/ui/UiInputSystem.hpp"

#include <optional>
#include <vector>

#include "engine/ui/Components.hpp"
#include "engine/ui/Layout.hpp"
#include "engine/ui/UiEvents.hpp"

namespace engine::ui {

core::Signature UiInputSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<UiTransform>());
	signature.set(coordinator.getComponentType<UiButton>());
	return signature;
}

/// Updates the hover and press state of every button from the pointer.
/// A click is a press then a release on the same button; clicks are published once every button is updated,
/// so a handler may create or destroy interface entities.
void UiInputSystem::update(core::Coordinator& coordinator, const input::IInputBackend& input, render::Point screen, core::EventBus& bus)
{
	const input::PointerState& pointer = input.pointer();
	const std::optional<core::Entity> underPointer = buttonUnderPointer(coordinator, pointer, screen);
	std::vector<UiClicked> clicks;
	for (core::Entity entity : entities) {
		auto& button = coordinator.getComponent<UiButton>(entity);
		button.hovered = underPointer == entity;
		if (pointer.pressed && button.hovered) {
			button.pressed = true;
		}
		if (pointer.released && button.pressed && button.hovered) {
			clicks.push_back({.entity = entity, .action = button.action});
		}
		if (!pointer.down) {
			button.pressed = false;
		}
	}
	for (const UiClicked& click : clicks) {
		bus.publish(click);
	}
}

/// Gives the button under the pointer with the highest layer, so only the one drawn on top reacts.
std::optional<core::Entity> UiInputSystem::buttonUnderPointer(core::Coordinator& coordinator, const input::PointerState& pointer, render::Point screen) const
{
	std::optional<core::Entity> found;
	int foundLayer = 0;
	for (core::Entity entity : entities) {
		const UiTransform& transform = coordinator.getComponent<UiTransform>(entity);
		if (!contains(resolveBounds(transform, screen), pointer.x, pointer.y)) {
			continue;
		}
		if (!found || transform.layer >= foundLayer) {
			found = entity;
			foundLayer = transform.layer;
		}
	}
	return found;
}

}
