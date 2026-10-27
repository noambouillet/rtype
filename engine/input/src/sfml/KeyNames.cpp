/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Key names used in settings files, mapped to SFML keys
*/

#include "KeyNames.hpp"

namespace engine::input {

/// Returns the SFML key for a settings file key name, or Unknown if the name is not listed.
sf::Keyboard::Key keyFromName(std::string_view name)
{
	for (const auto& entry : KEY_NAMES) {
		if (entry.first == name) {
			return entry.second;
		}
	}
	return sf::Keyboard::Key::Unknown;
}

}
