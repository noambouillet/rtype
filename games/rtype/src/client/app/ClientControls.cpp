/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** R-Type input actions
*/

#include "ClientControls.hpp"

#include <array>
#include <utility>

#include "engine/core/Logger.hpp"
#include "game/GameTypes.hpp"

namespace rtype::client {

using engine::input::AxisDirection;
using engine::input::JoystickAxis;

ClientControls::ClientControls() : _autoFire(false)
{
}

/// Returns the R-Type actions with their default keyboard and joystick bindings.
engine::input::InputMap ClientControls::defaultBindings()
{
	engine::input::InputMap map;
	map.bindKey(UP, "Up");
	map.bindAxis(UP, JoystickAxis::Y, AxisDirection::Negative);
	map.bindKey(DOWN, "Down");
	map.bindAxis(DOWN, JoystickAxis::Y, AxisDirection::Positive);
	map.bindKey(LEFT, "Left");
	map.bindAxis(LEFT, JoystickAxis::X, AxisDirection::Negative);
	map.bindKey(RIGHT, "Right");
	map.bindAxis(RIGHT, JoystickAxis::X, AxisDirection::Positive);
	map.bindKey(FIRE, "Space");
	map.bindButton(FIRE, 0);
	map.bindKey(AUTO_FIRE, "X");
	map.bindButton(AUTO_FIRE, 1);
	map.bindKey(QUIT, "Escape");
	map.bindKey(METRICS, "F3");
	engine::core::Logger::defaultLogger().debug("ClientControls: Default input bindings configured");
	return map;
}

/// Returns the button bit field to send to the server for this frame.
std::uint8_t ClientControls::buttons(const engine::input::IInputBackend& input)
{
	static constexpr std::array<std::pair<std::string_view, std::uint8_t>, 5> BUTTONS = {{
		{UP, game::BUTTON_UP},
		{DOWN, game::BUTTON_DOWN},
		{LEFT, game::BUTTON_LEFT},
		{RIGHT, game::BUTTON_RIGHT},
		{FIRE, game::BUTTON_FIRE},
	}};

	if (input.justPressed(AUTO_FIRE)) {
		_autoFire = !_autoFire;
	}
	std::uint8_t value = _autoFire ? game::BUTTON_FIRE : std::uint8_t{0};
	for (const auto& [name, bit] : BUTTONS) {
		if (input.isDown(name)) {
			value |= bit;
		}
	}
	return value;
}

}
