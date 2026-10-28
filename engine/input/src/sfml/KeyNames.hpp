/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Key names used in settings files, mapped to SFML keys
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <string_view>
#include <utility>

namespace engine::input {

static constexpr std::array<std::pair<std::string_view, sf::Keyboard::Key>, 33> KEY_NAMES = {{
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
}};

sf::Keyboard::Key keyFromName(std::string_view name);

}
