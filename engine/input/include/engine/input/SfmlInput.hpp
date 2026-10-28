/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML keyboard and joystick backend
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "engine/input/InputBackend.hpp"
#include "engine/input/InputMap.hpp"

namespace engine::input {

class SfmlInput : public IInputBackend {
public:
	explicit SfmlInput(InputMap map, std::uint32_t joystick = 0);

	void update(bool keyboardFocused) override;
	bool isDown(std::string_view action) const override;
	bool justPressed(std::string_view action) const override;

private:
	struct State {
		bool down = false;
		bool wasDown = false;
	};

	std::optional<std::size_t> indexOf(std::string_view action) const;
	static bool keyDown(const ActionBinding& binding);
	bool joystickDown(const ActionBinding& binding) const;

	InputMap _map;
	std::uint32_t _joystickId;
	std::vector<State> _states;
};

}
