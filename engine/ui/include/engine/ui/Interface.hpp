/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Setup and frame steps of the interface systems
*/

#pragma once

#include "engine/core/EventBus.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/input/InputBackend.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace engine::ui {

void registerInterface(core::Coordinator& coordinator);
void updateInterface(core::Coordinator& coordinator, const input::IInputBackend& input, const render::IGraphicsBackend& backend, core::EventBus& bus);
void renderInterface(core::Coordinator& coordinator, render::IGraphicsBackend& backend);

}
