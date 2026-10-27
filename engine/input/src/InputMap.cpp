/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Action to device bindings
*/

#include "engine/input/InputMap.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace engine::input {

InputMap::InputMap() : _bindings{}, _deadzone(DEFAULT_JOYSTICK_DEADZONE)
{
}

/// Returns the binding of the action, adding an empty one if the action is new.
ActionBinding& InputMap::findOrAdd(std::string_view action)
{
	for (ActionBinding& binding : _bindings) {
		if (binding.name == action)
			return binding;
	}
	return _bindings.emplace_back(ActionBinding{std::string(action), "", std::nullopt, std::nullopt});
}

/// Binds the action to a key name such as "Space", replacing its previous key.
void InputMap::bindKey(std::string_view action, std::string key)
{
	findOrAdd(action).key = std::move(key);
}

/// Binds the action to a joystick button, replacing its previous button.
void InputMap::bindButton(std::string_view action, std::uint32_t button)
{
	findOrAdd(action).button = button;
}

/// Binds the action to one direction of a joystick axis, replacing its previous axis.
void InputMap::bindAxis(std::string_view action, JoystickAxis axis, AxisDirection direction)
{
	findOrAdd(action).axis = AxisBinding{axis, direction};
}

/// Matches an action name and key name to update the corresponding keyboard binding.
void InputMap::parseKeyboardBinding(
	std::string_view actionKey,
	const std::string& keyName,
	std::vector<ActionBinding>& bindings)
{
	for (ActionBinding& binding : bindings) {
		if (binding.name == actionKey) {
			binding.key = keyName;
			return;
		}
	}
}

/// Parses and validates a joystick configuration value, discarding invalid numbers.
void InputMap::parseJoystickSetting(
	std::string_view key,
	const std::string& value,
	std::uint32_t& deadzone,
	std::vector<ActionBinding>& bindings)
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
		return;
	}
	const std::string_view action = key.substr(JOYSTICK_PREFIX.size());
	for (ActionBinding& binding : bindings) {
		if (binding.name == action) {
			binding.button = number;
			return;
		}
	}
}

/// Loads key bindings and joystick settings from a key-value map, ignoring unrecognized or invalid entries.
void InputMap::loadFromMap(const std::map<std::string, std::string>& data)
{
	auto bindings = _bindings;
	std::uint32_t deadzoneValue = _deadzone;

	for (const auto& [key, value] : data) {
		if (key.starts_with(JOYSTICK_PREFIX)) {
			parseJoystickSetting(key, value, deadzoneValue, bindings);
		} else {
			parseKeyboardBinding(key, value, bindings);
		}
	}
	_bindings = bindings;
	_deadzone = deadzoneValue;
}

/// Serializes the current key bindings and joystick settings to a key-value map.
void InputMap::saveToMap(std::map<std::string, std::string>& data) const
{
	for (const ActionBinding& binding : _bindings) {
		if (!binding.key.empty())
			data[binding.name] = binding.key;
		if (binding.button)
			data[std::string(JOYSTICK_PREFIX) + binding.name] = std::to_string(*binding.button);
	}
	data["joystick_deadzone"] = std::to_string(_deadzone);
}

/// Returns every bound action, in the order of their first bind.
const std::vector<ActionBinding>& InputMap::actions() const
{
	return _bindings;
}

/// Returns the current joystick deadzone percentage threshold.
std::uint32_t InputMap::deadzone() const
{
	return _deadzone;
}

}
