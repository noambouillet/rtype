/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Action to device bindings
*/

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace engine::input {

enum class JoystickAxis : std::uint8_t {
	X,
	Y,
};

enum class AxisDirection : std::uint8_t {
	Negative,
	Positive,
};

struct AxisBinding {
	JoystickAxis axis;
	AxisDirection direction;
};

struct ActionBinding {
	std::string name;
	std::string key;
	std::optional<std::uint32_t> button;
	std::optional<AxisBinding> axis;
};

constexpr std::uint32_t DEFAULT_JOYSTICK_DEADZONE = 30;

class InputMap {
public:
	InputMap();

	void bindKey(std::string_view action, std::string key);
	void bindButton(std::string_view action, std::uint32_t button);
	void bindAxis(std::string_view action, JoystickAxis axis, AxisDirection direction);

	void loadFromMap(const std::map<std::string, std::string>& data);
	void saveToMap(std::map<std::string, std::string>& data) const;

	const std::vector<ActionBinding>& actions() const;
	std::uint32_t deadzone() const;

private:
	ActionBinding& findOrAdd(std::string_view action);
	void parseKeyboardBinding(std::string_view actionKey, const std::string& keyName, std::vector<ActionBinding>& bindings);
	void parseJoystickSetting(std::string_view key, const std::string& value, std::uint32_t& deadzone, std::vector<ActionBinding>& bindings);

	std::vector<ActionBinding> _bindings;
	std::uint32_t _deadzone;
};

}
