/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML keyboard and joystick backend
*/

#include "engine/input/SfmlInput.hpp"

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <utility>

namespace engine::input {

static const std::vector<std::pair<std::string_view, sf::Keyboard::Key>> KEY_NAMES = {
	{"Up", sf::Keyboard::Key::Up},
	{"Down", sf::Keyboard::Key::Down},
	{"Left", sf::Keyboard::Key::Left},
	{"Right", sf::Keyboard::Key::Right},
	{"Space", sf::Keyboard::Key::Space},
	{"Escape", sf::Keyboard::Key::Escape},
	{"Enter", sf::Keyboard::Key::Enter},
	{"A", sf::Keyboard::Key::A},
	{"B", sf::Keyboard::Key::B},
	{"C", sf::Keyboard::Key::C},
	{"D", sf::Keyboard::Key::D},
	{"E", sf::Keyboard::Key::E},
	{"F", sf::Keyboard::Key::F},
	{"G", sf::Keyboard::Key::G},
	{"H", sf::Keyboard::Key::H},
	{"I", sf::Keyboard::Key::I},
	{"J", sf::Keyboard::Key::J},
	{"K", sf::Keyboard::Key::K},
	{"L", sf::Keyboard::Key::L},
	{"M", sf::Keyboard::Key::M},
	{"N", sf::Keyboard::Key::N},
	{"O", sf::Keyboard::Key::O},
	{"P", sf::Keyboard::Key::P},
	{"Q", sf::Keyboard::Key::Q},
	{"R", sf::Keyboard::Key::R},
	{"S", sf::Keyboard::Key::S},
	{"T", sf::Keyboard::Key::T},
	{"U", sf::Keyboard::Key::U},
	{"V", sf::Keyboard::Key::V},
	{"W", sf::Keyboard::Key::W},
	{"X", sf::Keyboard::Key::X},
	{"Y", sf::Keyboard::Key::Y},
	{"Z", sf::Keyboard::Key::Z},
};

/// Returns the SFML key for a settings file key name, or Unknown if the name is not listed.
static sf::Keyboard::Key keyFromName(std::string_view name)
{
	for (const auto& entry : KEY_NAMES) {
		if (entry.first == name) {
			return entry.second;
		}
	}
	return sf::Keyboard::Key::Unknown;
}

/// Keeps a copy of the map, so every action must be bound before creating the backend.
SfmlInput::SfmlInput(InputMap map, std::uint32_t joystick)
	: _map(std::move(map)), _joystickId(joystick), _states(_map.actions().size())
{
}

/// Reads the devices for every action.
void SfmlInput::update(bool keyboardFocused)
{
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
	const float threshold = static_cast<float>(_map.deadzone());
	return binding.axis->direction == AxisDirection::Negative ? position < -threshold : position > threshold;
}

}
