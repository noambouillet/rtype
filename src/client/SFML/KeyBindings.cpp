/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML-specific client settings
*/

#include "KeyBindings.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

namespace rtype {

KeyBindings::KeyBindings()
	: _bindings{
		  sf::Keyboard::Key::Up,
		  sf::Keyboard::Key::Down,
		  sf::Keyboard::Key::Left,
		  sf::Keyboard::Key::Right,
		  sf::Keyboard::Key::Space,
		  sf::Keyboard::Key::X},
	  _deadzone(30),
	  _joystickFire(0),
	  _joystickAutoFire(1)
{
}

void KeyBindings::loadFromMap(const std::map<std::string, std::string>& data)
{
	auto bindings = _bindings;
	std::uint32_t deadzoneValue = _deadzone;
	std::uint32_t joystickFireValue = _joystickFire;
	std::uint32_t joystickAutoFireValue = _joystickAutoFire;

	for (const auto& [key, value] : data) {
		const auto action = std::find(ACTION_NAMES.begin(), ACTION_NAMES.end(), key);
		if (action != ACTION_NAMES.end()) {
			bool keyFound = false;
			for (const auto& entry : KEY_NAMES) {
				if (entry.first == value) {
					const std::size_t index = static_cast<std::size_t>(std::distance(ACTION_NAMES.begin(), action));
					bindings[index] = entry.second;
					keyFound = true;
					break;
				}
			}
			if (!keyFound)
				continue;
			continue;
		}

		if (key == "joystick_deadzone" || key == "joystick_fire" || key == "joystick_auto_fire") {
			if (value.empty() || value.front() == '-')
				continue;
			unsigned long parsedValue = 0;
			try {
				std::size_t parsedLength = 0;
				parsedValue = std::stoul(value, &parsedLength);
				if (parsedLength != value.size())
					continue;
			} catch (const std::invalid_argument&) {
				continue;
			} catch (const std::out_of_range&) {
				continue;
			}
			if (parsedValue > std::numeric_limits<std::uint32_t>::max()) {
				continue;
			}
			const std::uint32_t number = static_cast<std::uint32_t>(parsedValue);
			if (key == "joystick_deadzone") {
				if (number > 100) {
					continue;
				}
				deadzoneValue = number;
			} else if (key == "joystick_fire") {
				joystickFireValue = number;
			} else {
				joystickAutoFireValue = number;
			}
			continue;
		}
	}
	_bindings = bindings;
	_deadzone = deadzoneValue;
	_joystickFire = joystickFireValue;
	_joystickAutoFire = joystickAutoFireValue;
}

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

sf::Keyboard::Key KeyBindings::getBinding(ActionType action) const
{
	return _bindings[static_cast<std::size_t>(action)];
}

sf::Keyboard::Key KeyBindings::getBinding(std::string_view action) const
{
	for (std::size_t i = 0; i < ACTION_NAMES.size(); i++) {
		if (ACTION_NAMES[i] == action) {
			return _bindings[i];
		}
	}
	return sf::Keyboard::Key::Unknown;
}

std::uint32_t KeyBindings::deadzone() const
{
	return _deadzone;
}

std::uint32_t KeyBindings::joystickFire() const
{
	return _joystickFire;
}

std::uint32_t KeyBindings::joystickAutoFire() const
{
	return _joystickAutoFire;
}

}
