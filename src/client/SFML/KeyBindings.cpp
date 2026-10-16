/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Configurable keyboard and joystick bindings
*/

#include "KeyBindings.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

namespace rtype {

/// Removes surrounding whitespace from a configuration line or value.
std::string_view trim(std::string_view value)
{
	const std::size_t first = value.find_first_not_of(" \t\r\n");
	if (first == std::string_view::npos)
		return {};
	const std::size_t last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

/// Initializes keyboard bindings and joystick settings to their defaults.
KeyBindings::KeyBindings()
	: _bindings{
		  sf::Keyboard::Key::Up,
		  sf::Keyboard::Key::Down,
		  sf::Keyboard::Key::Left,
		  sf::Keyboard::Key::Right,
		  sf::Keyboard::Key::Space,
		  sf::Keyboard::Key::X},
	  _deadzone(30),
	  _joystickFire(0)
{
}

/// Resolves an action and key name, then updates the staged keyboard bindings.
bool KeyBindings::parseActionLine(std::string_view key, std::string_view value,
	std::array<sf::Keyboard::Key, key_bindings_config::ACTION_COUNT>& bindings) const
{
	std::vector<std::string_view>::const_iterator actionNamePosition =
		std::find(key_bindings_config::ACTION_NAMES.begin(), key_bindings_config::ACTION_NAMES.end(), key);
	if (actionNamePosition == key_bindings_config::ACTION_NAMES.end())
		return false;
	std::size_t actionIndex = std::distance(key_bindings_config::ACTION_NAMES.begin(), actionNamePosition);
	for (const auto& [name, keyCode] : key_bindings_config::KEY_NAMES) {
		if (name == value) {
			bindings[actionIndex] = keyCode;
			return true;
		}
	}
	return false;
}

/// Parses an unsigned joystick setting and validates the axis deadzone range.
bool KeyBindings::parseJoystickLine(std::string_view key, std::string_view value,
	std::uint32_t& deadzoneValue, std::uint32_t& joystickFireValue) const
{
	const std::string numberText(value);
	if (numberText.empty() || numberText.front() == '-')
		return false;
	unsigned long parsedNumber = 0;
	std::size_t parsedLength = 0;
	try {
		parsedNumber = std::stoul(numberText, &parsedLength);
	} catch (const std::invalid_argument&) {
		return false;
	} catch (const std::out_of_range&) {
		return false;
	}
	if (parsedLength != numberText.size() || parsedNumber > std::numeric_limits<std::uint32_t>::max())
		return false;
	const auto number = static_cast<std::uint32_t>(parsedNumber);

	if (key == "joystick_deadzone") {
		if (number > 100)
			return false;
		deadzoneValue = number;
		return true;
	}
	if (key == "joystick_fire") {
		joystickFireValue = number;
		return true;
	}
	return false;
}

/// Processes one configuration line and delegates it to its matching parser.
bool KeyBindings::parseLine(std::string_view line, std::array<sf::Keyboard::Key, key_bindings_config::ACTION_COUNT>& bindings,
	std::uint32_t& deadzoneValue, std::uint32_t& joystickFireValue) const
{
	const std::string_view content = trim(line);
	if (content.empty() || content.front() == '#')
		return true;

	const std::size_t separator = content.find('=');
	if (separator == std::string_view::npos)
		return false;
	const std::string_view key = trim(content.substr(0, separator));
	const std::string_view value = trim(content.substr(separator + 1));
	if (key.empty() || value.empty())
		return false;

	for (const std::string_view action : key_bindings_config::ACTION_NAMES) {
		if (key == action)
			return parseActionLine(key, value, bindings);
	}
	if (key == "joystick_deadzone" || key == "joystick_fire")
		return parseJoystickLine(key, value, deadzoneValue, joystickFireValue);
	return false;
}

/// Reads and validates each line before applying the loaded settings.
bool KeyBindings::load(const std::string& filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open())
		return false;

	auto bindings = _bindings;
	std::uint32_t deadzoneValue = _deadzone;
	std::uint32_t joystickFireValue = _joystickFire;
	std::string line;
	while (std::getline(file, line)) {
		if (!parseLine(line, bindings, deadzoneValue, joystickFireValue)) {
			continue;
		}
	}
	_bindings = bindings;
	_deadzone = deadzoneValue;
	_joystickFire = joystickFireValue;
	return true;
}

/// Writes the current keyboard and joystick settings to a configuration file.
bool KeyBindings::save(const std::string& filepath) const
{
	std::ofstream file(filepath);
	if (!file.is_open())
		return false;

	for (std::size_t i = 0; i < key_bindings_config::ACTION_NAMES.size(); i++) {
		for (const auto& [name, key] : key_bindings_config::KEY_NAMES) {
			if (key == _bindings[i]) {
				file << key_bindings_config::ACTION_NAMES[i] << " = " << name << '\n';
				break;
			}
		}
	}
	file << "joystick_fire = " << _joystickFire << '\n';
	file << "joystick_deadzone = " << _deadzone << '\n';
	file.close();
	return !file.fail();
}

/// Returns the binding indexed by an action enum for gameplay lookup.
sf::Keyboard::Key KeyBindings::getBinding(ActionType action) const
{
	return _bindings[static_cast<std::size_t>(action)];
}

/// Returns the binding matching an action name, or Unknown when not found.
sf::Keyboard::Key KeyBindings::getBinding(std::string_view action) const
{
	for (std::size_t i = 0; i < key_bindings_config::ACTION_NAMES.size(); i++) {
		if (key_bindings_config::ACTION_NAMES[i] == action) {
			return _bindings[i];
		}
	}
	return sf::Keyboard::Key::Unknown;
}

/// Returns the configured joystick axis deadzone.
std::uint32_t KeyBindings::deadzone() const
{
	return _deadzone;
}

/// Returns the configured joystick fire button index.
std::uint32_t KeyBindings::joystickFire() const
{
	return _joystickFire;
}

}
