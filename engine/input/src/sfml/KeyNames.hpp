/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Key names used in settings files, mapped to SFML keys
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <string_view>
#include <utility>
#include <vector>

namespace engine::input {

extern const std::vector<std::pair<std::string_view, sf::Keyboard::Key>> KEY_NAMES;

sf::Keyboard::Key keyFromName(std::string_view name);

}
