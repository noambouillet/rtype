/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface input system
*/

#include "engine/ui/UiInputSystem.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "engine/ui/Components.hpp"
#include "engine/ui/Layout.hpp"
#include "engine/ui/UiEvents.hpp"

namespace engine::ui {

UiInputSystem::UiInputSystem()
	: _previousAction(), _nextAction(), _confirmAction(), _focused(std::nullopt), _keyboardMode(false), _lastPointerX(0.0F), _lastPointerY(0.0F)
{
}

core::Signature UiInputSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<UiTransform>());
	signature.set(coordinator.getComponentType<UiButton>());
	return signature;
}

/// Names the input actions that move the focus to the previous or next UiFocusable button and click the focused one.
void UiInputSystem::setNavigation(std::string previous, std::string next, std::string confirm)
{
	_previousAction = std::move(previous);
	_nextAction = std::move(next);
	_confirmAction = std::move(confirm);
}

/// Updates the hover and press state of every button, from the pointer or from the focus.
/// Moving or clicking the mouse gives control to the pointer; a navigation action gives it to the focus.
void UiInputSystem::update(core::Coordinator& coordinator, const input::IInputBackend& input, render::Point screen, core::EventBus& bus)
{
	const input::PointerState& pointer = input.pointer();
	if (pointer.x != _lastPointerX || pointer.y != _lastPointerY || pointer.pressed) {
		_keyboardMode = false;
	}
	_lastPointerX = pointer.x;
	_lastPointerY = pointer.y;
	if (!_previousAction.empty() && input.justPressed(_previousAction)) {
		moveFocus(coordinator, -1);
	}
	if (!_nextAction.empty() && input.justPressed(_nextAction)) {
		moveFocus(coordinator, 1);
	}
	const std::optional<core::Entity> underPointer = buttonUnderPointer(coordinator, pointer, screen);
	if (!_keyboardMode && underPointer && coordinator.hasComponent<UiFocusable>(*underPointer)) {
		_focused = underPointer;
	}
	std::vector<UiClicked> clicks;
	for (core::Entity entity : entities) {
		auto& button = coordinator.getComponent<UiButton>(entity);
		button.hovered = _keyboardMode ? _focused == entity : underPointer == entity;
		if (pointer.pressed && button.hovered && !_keyboardMode) {
			button.pressed = true;
		}
		if (pointer.released && button.pressed && button.hovered) {
			clicks.push_back({.entity = entity, .action = button.action});
		}
		if (!pointer.down) {
			button.pressed = false;
		}
	}
	const std::optional<core::Entity> focused = focusedButton(coordinator);
	if (_keyboardMode && focused && !_confirmAction.empty() && input.justPressed(_confirmAction)) {
		clicks.push_back({.entity = *focused, .action = coordinator.getComponent<UiButton>(*focused).action});
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

/// Moves the focus to the previous or next UiFocusable button by order, wrapping around.
void UiInputSystem::moveFocus(core::Coordinator& coordinator, int step)
{
	std::vector<core::Entity> focusables;
	for (core::Entity entity : entities) {
		if (coordinator.hasComponent<UiFocusable>(entity)) {
			focusables.push_back(entity);
		}
	}
	if (focusables.empty()) {
		return;
	}
	std::ranges::stable_sort(focusables, [&coordinator](core::Entity left, core::Entity right) {
		return coordinator.getComponent<UiFocusable>(left).order < coordinator.getComponent<UiFocusable>(right).order;
	});
	const std::optional<core::Entity> focused = focusedButton(coordinator);
	_keyboardMode = true;
	const auto current = focused ? std::ranges::find(focusables, *focused) : focusables.end();
	if (current == focusables.end()) {
		_focused = focusables.front();
		return;
	}
	const auto count = static_cast<int>(focusables.size());
	const int index = (static_cast<int>(current - focusables.begin()) + step + count) % count;
	_focused = focusables[static_cast<std::size_t>(index)];
}

/// Gives the focused button, or nothing if there is no focus or the focused entity is no longer a focusable button.
std::optional<core::Entity> UiInputSystem::focusedButton(core::Coordinator& coordinator) const
{
	if (!_focused || !entities.contains(*_focused) || !coordinator.hasComponent<UiFocusable>(*_focused)) {
		return std::nullopt;
	}
	return _focused;
}

}
