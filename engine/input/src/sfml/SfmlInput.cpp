/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML keyboard and joystick backend
*/

#include "engine/input/SfmlInput.hpp"

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <utility>

#include "KeyNames.hpp"

namespace engine::input {

/// Keeps a copy of the map, so every action must be bound before creating the backend.
SfmlInput::SfmlInput(InputMap map, std::uint32_t joystick)
	: _map(std::move(map)), _joystickId(joystick), _states(_map.actions().size()), _pointer{.x = 0.0F, .y = 0.0F, .down = false, .pressed = false, .released = false}
{
}

/// Reads the devices for every action, and the left mouse button while the window has the focus.
void SfmlInput::update(bool keyboardFocused)
{
	const bool wasDown = _pointer.down;
	_pointer.down = keyboardFocused && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	_pointer.pressed = _pointer.down && !wasDown;
	_pointer.released = !_pointer.down && wasDown;
	const std::vector<ActionBinding>& actions = _map.actions();
	for (std::size_t index = 0; index < actions.size(); index++) {
		State& state = _states[index];
		state.wasDown = state.down;
		state.down = (keyboardFocused && keyDown(actions[index])) || joystickDown(actions[index]);
	}
}

/// Tells whether the action was held at the last update. Unknown actions are never down.
bool SfmlInput::isDown(std::string_view action) const
{
	const auto index = indexOf(action);
	return index && _states[*index].down;
}

/// Tells whether the action was pressed at the last update, so it is true once per press.
bool SfmlInput::justPressed(std::string_view action) const
{
	const auto index = indexOf(action);
	return index && _states[*index].down && !_states[*index].wasDown;
}

const PointerState& SfmlInput::pointer() const
{
	return _pointer;
}

/// Stores the cursor position, which the window gives in the coordinates the interface is drawn in.
void SfmlInput::setPointerPosition(PointerPosition position)
{
	_pointer.x = position.x;
	_pointer.y = position.y;
}

std::optional<std::size_t> SfmlInput::indexOf(std::string_view action) const
{
	const std::vector<ActionBinding>& actions = _map.actions();
	for (std::size_t index = 0; index < actions.size(); index++) {
		if (actions[index].name == action) {
			return index;
		}
	}
	return std::nullopt;
}

bool SfmlInput::keyDown(const ActionBinding& binding)
{
	const sf::Keyboard::Key key = keyFromName(binding.key);
	return key != sf::Keyboard::Key::Unknown && sf::Keyboard::isKeyPressed(key);
}

/// Tells whether the action's joystick button is held or its axis is pushed past the deadzone.
bool SfmlInput::joystickDown(const ActionBinding& binding) const
{
	if (!sf::Joystick::isConnected(_joystickId)) {
		return false;
	}
	const unsigned int buttonCount = sf::Joystick::getButtonCount(_joystickId);
	if (binding.button && *binding.button < buttonCount && sf::Joystick::isButtonPressed(_joystickId, *binding.button)) {
		return true;
	}
	if (!binding.axis) {
		return false;
	}
	const sf::Joystick::Axis axis = binding.axis->axis == JoystickAxis::X ? sf::Joystick::Axis::X : sf::Joystick::Axis::Y;
	const float position = sf::Joystick::getAxisPosition(_joystickId, axis);
	const auto threshold = static_cast<float>(_map.deadzone());
	return binding.axis->direction == AxisDirection::Negative ? position < -threshold : position > threshold;
}

}
