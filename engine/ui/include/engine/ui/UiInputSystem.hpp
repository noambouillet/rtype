/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Interface input system
*/

#pragma once

#include <optional>

#include "engine/core/EventBus.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"
#include "engine/input/InputBackend.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace engine::ui {

class UiInputSystem : public core::System {
public:
	static core::Signature signature(core::Coordinator& coordinator);
	void update(core::Coordinator& coordinator, const input::IInputBackend& input, render::Point screen, core::EventBus& bus);

private:
	std::optional<core::Entity> buttonUnderPointer(core::Coordinator& coordinator, const input::PointerState& pointer, render::Point screen) const;
};

}
