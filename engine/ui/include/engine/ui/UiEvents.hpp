/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Events published on the EventBus by the interface
*/

#pragma once

#include <string>

#include "engine/core/ecs/Entity.hpp"

namespace engine::ui {

struct UiClicked {
	core::Entity entity;
	std::string action;
};

}
