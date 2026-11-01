/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface input system
*/

#pragma once

#include <optional>
#include <string>

#include "engine/core/EventBus.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/input/InputBackend.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace engine::ui {

class UiInputSystem : public core::System {
public:
	UiInputSystem();
	static core::Signature signature(core::Coordinator& coordinator);
	void setNavigation(std::string previous, std::string next, std::string confirm);
	void update(core::Coordinator& coordinator, const input::IInputBackend& input, render::Point screen, core::EventBus& bus);

private:
	std::optional<core::Entity> buttonUnderPointer(core::Coordinator& coordinator, const input::PointerState& pointer, render::Point screen) const;
	void moveFocus(core::Coordinator& coordinator, int step);
	std::optional<core::Entity> focusedButton(core::Coordinator& coordinator) const;

	std::string _previousAction;
	std::string _nextAction;
	std::string _confirmAction;
	std::optional<core::Entity> _focused;
	bool _keyboardMode;
	float _lastPointerX;
	float _lastPointerY;
};

}
