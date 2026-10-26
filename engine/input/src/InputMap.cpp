/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML-specific client settings
*/

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

#include "KeyBindings.hpp"

namespace rtype {

/// Initializes the default keyboard bindings and joystick settings.
KeyBindings::KeyBindings()
	: _bindings{
		  DEFAULT_KEY_UP,
		  DEFAULT_KEY_DOWN,
		  DEFAULT_KEY_LEFT,
		  DEFAULT_KEY_RIGHT,
		  DEFAULT_KEY_FIRE,
		  DEFAULT_KEY_AUTO_FIRE},
	  _deadzone(DEFAULT_JOYSTICK_DEADZONE),
	  _joystickFire(DEFAULT_JOYSTICK_FIRE),
	  _joystickAutoFire(DEFAULT_JOYSTICK_AUTO_FIRE)
{
}

/// Matches an action name and key name to update the corresponding keyboard binding.
void KeyBindings::parseKeyboardBinding(
	std::string_view actionKey,
	const std::string& keyName,
	std::vector<sf::Keyboard::Key>& bindings)
{
	const auto action = std::find(ACTION_NAMES.begin(), ACTION_NAMES.end(), actionKey);
	if (action == ACTION_NAMES.end())
		return;
	for (const auto& entry : KEY_NAMES) {
		if (entry.first == keyName) {
			const std::size_t index = static_cast<std::size_t>(std::distance(ACTION_NAMES.begin(), action));
			bindings[index] = entry.second;
			return;
		}
	}
}

/// Parses and validates a joystick configuration value, discarding invalid numbers.
void KeyBindings::parseJoystickSetting(
	std::string_view key,
	const std::string& value,
	std::uint32_t& deadzone,
	std::uint32_t& fire,
	std::uint32_t& autoFire)
{
	if (value.empty() || value.front() == '-')
		return;
	unsigned long parsedValue = 0;
	try {
		std::size_t parsedLength = 0;
		parsedValue = std::stoul(value, &parsedLength);
		if (parsedLength != value.size())
			return;
	} catch (const std::invalid_argument&) {
		return;
	} catch (const std::out_of_range&) {
		return;
	}
	if (parsedValue > std::numeric_limits<std::uint32_t>::max())
		return;
	const std::uint32_t number = static_cast<std::uint32_t>(parsedValue);
	if (key == "joystick_deadzone") {
		if (number <= 100)
			deadzone = number;
	} else if (key == "joystick_fire") {
		fire = number;
	} else if (key == "joystick_auto_fire") {
		autoFire = number;
	}
}

/// Loads key bindings and joystick settings from a key-value map, ignoring unrecognized or invalid entries.
void KeyBindings::loadFromMap(const std::map<std::string, std::string>& data)
{
	auto bindings = _bindings;
	std::uint32_t deadzoneValue = _deadzone;
	std::uint32_t joystickFireValue = _joystickFire;
	std::uint32_t joystickAutoFireValue = _joystickAutoFire;

	for (const auto& [key, value] : data) {
		if (key == "joystick_deadzone" || key == "joystick_fire" || key == "joystick_auto_fire") {
			parseJoystickSetting(key, value, deadzoneValue, joystickFireValue, joystickAutoFireValue);
		} else {
			parseKeyboardBinding(key, value, bindings);
		}
	}
	_bindings = bindings;
	_deadzone = deadzoneValue;
	_joystickFire = joystickFireValue;
	_joystickAutoFire = joystickAutoFireValue;
}

/// Serializes the current key bindings and joystick settings to a key-value map.
void KeyBindings::saveToMap(std::map<std::string, std::string>& data) const
{
	for (std::size_t i = 0; i < ACTION_NAMES.size(); i++) {
		for (const auto& entry : KEY_NAMES) {
			if (entry.second == _bindings[i]) {
				data[std::string(ACTION_NAMES[i])] = std::string(entry.first);
				break;
			}
		}
	}
	data["joystick_deadzone"] = std::to_string(_deadzone);
	data["joystick_fire"] = std::to_string(_joystickFire);
	data["joystick_auto_fire"] = std::to_string(_joystickAutoFire);
}

/// Returns the key currently bound to the specified action enum value.
sf::Keyboard::Key KeyBindings::getBinding(ActionType action) const
{
	return _bindings[static_cast<std::size_t>(action)];
}

/// Returns the key currently bound to the specified action name string, or Unknown if not found.
sf::Keyboard::Key KeyBindings::getBinding(std::string_view action) const
{
	for (std::size_t i = 0; i < ACTION_NAMES.size(); i++) {
		if (ACTION_NAMES[i] == action) {
			return _bindings[i];
		}
	}
	return sf::Keyboard::Key::Unknown;
}

/// Returns the current joystick deadzone percentage threshold.
std::uint32_t KeyBindings::deadzone() const
{
	return _deadzone;
}

/// Returns the joystick button index assigned to regular firing.
std::uint32_t KeyBindings::joystickFire() const
{
	return _joystickFire;
}

/// Returns the joystick button index assigned to auto-firing.
std::uint32_t KeyBindings::joystickAutoFire() const
{
	return _joystickAutoFire;
}

}
