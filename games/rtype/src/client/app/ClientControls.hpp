/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** R-Type input actions
*/

#pragma once

#include <cstdint>
#include <string_view>

#include "engine/input/InputBackend.hpp"
#include "engine/input/InputMap.hpp"

namespace rtype::client {

class ClientControls {
public:
	static constexpr std::string_view UP = "up";
	static constexpr std::string_view DOWN = "down";
	static constexpr std::string_view LEFT = "left";
	static constexpr std::string_view RIGHT = "right";
	static constexpr std::string_view FIRE = "fire";
	static constexpr std::string_view AUTO_FIRE = "auto_fire";
	static constexpr std::string_view PAUSE = "pause";
	static constexpr std::string_view UI_UP = "ui_up";
	static constexpr std::string_view UI_DOWN = "ui_down";
	static constexpr std::string_view UI_CONFIRM = "ui_confirm";
	static constexpr std::string_view UI_BACK = "ui_back";
	static constexpr std::string_view METRICS = "metrics";

	static constexpr std::uint32_t JOYSTICK_CONFIRM = 0;
	static constexpr std::uint32_t JOYSTICK_BACK = 1;
	static constexpr std::uint32_t JOYSTICK_START = 7;

	ClientControls();
	static engine::input::InputMap defaultBindings();
	std::uint8_t buttons(const engine::input::IInputBackend& input);

private:
	bool _autoFire;
};

}
