/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Button style system
*/

#pragma once

#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace engine::ui {

class UiButtonStyleSystem : public core::System {
public:
	static core::Signature signature(core::Coordinator& coordinator);
	void update(core::Coordinator& coordinator);
};

}
