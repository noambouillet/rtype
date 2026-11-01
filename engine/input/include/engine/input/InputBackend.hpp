/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Input backend interface
*/

#pragma once

#include <string_view>

namespace engine::input {

struct PointerPosition {
	float x;
	float y;
};

struct PointerState {
	float x;
	float y;
	bool down;
	bool pressed;
	bool released;
};

class IInputBackend {
public:
	virtual ~IInputBackend() = default;
	virtual void update(bool keyboardFocused) = 0;
	virtual bool isDown(std::string_view action) const = 0;
	virtual bool justPressed(std::string_view action) const = 0;
	virtual const PointerState& pointer() const = 0;
};

}
