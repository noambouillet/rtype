/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML-specific client settings
*/

#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace rtype {

enum class ActionType {
	Up,
	Down,
	Left,
	Right,
	Fire,
	AutoFire,
};

constexpr std::size_t ACTION_COUNT = 6;

inline const std::vector<std::string_view> ACTION_NAMES = {
	"up",
	"down",
	"left",
	"right",
	"fire",
	"auto_fire",
};

inline const std::vector<std::pair<std::string_view, sf::Keyboard::Key>> KEY_NAMES = {
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

class KeyBindings {
public:
	KeyBindings();

	void loadFromMap(const std::map<std::string, std::string>& data);
	void saveToMap(std::map<std::string, std::string>& data) const;

	sf::Keyboard::Key getBinding(ActionType action) const;
	sf::Keyboard::Key getBinding(std::string_view action) const;
	std::uint32_t deadzone() const;
	std::uint32_t joystickFire() const;
	std::uint32_t joystickAutoFire() const;

private:
	std::vector<sf::Keyboard::Key> _bindings;
	std::uint32_t _deadzone;
	std::uint32_t _joystickFire;
	std::uint32_t _joystickAutoFire;
};

}
