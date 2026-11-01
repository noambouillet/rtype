/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Button style system
*/

#include "engine/ui/UiButtonStyleSystem.hpp"

#include "engine/ui/Components.hpp"

namespace engine::ui {

/// Gives the value matching the state of a button: pressed first, then hovered, then normal.
template <typename Value>
static Value stateValue(const UiButton& button, const Value& normal, const Value& hovered, const Value& pressed)
{
	if (button.pressed) {
		return pressed;
	}
	if (button.hovered) {
		return hovered;
	}
	return normal;
}

core::Signature UiButtonStyleSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<UiButton>());
	return signature;
}

/// Shows the state of every button: a UiButtonStyle picks the image region, a UiColorStyle picks the rectangle color.
void UiButtonStyleSystem::update(core::Coordinator& coordinator)
{
	for (core::Entity entity : entities) {
		const UiButton& button = coordinator.getComponent<UiButton>(entity);
		if (coordinator.hasComponent<UiButtonStyle>(entity) && coordinator.hasComponent<UiImage>(entity)) {
			const UiButtonStyle& style = coordinator.getComponent<UiButtonStyle>(entity);
			coordinator.getComponent<UiImage>(entity).region = stateValue(button, style.normal, style.hovered, style.pressed);
		}
		if (coordinator.hasComponent<UiColorStyle>(entity) && coordinator.hasComponent<UiRect>(entity)) {
			const UiColorStyle& style = coordinator.getComponent<UiColorStyle>(entity);
			coordinator.getComponent<UiRect>(entity).color = stateValue(button, style.normal, style.hovered, style.pressed);
		}
	}
}

}
