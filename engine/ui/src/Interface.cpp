/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Setup and frame steps of the interface systems
*/

#include "engine/ui/Interface.hpp"

#include "engine/ui/Components.hpp"
#include "engine/ui/UiButtonStyleSystem.hpp"
#include "engine/ui/UiInputSystem.hpp"
#include "engine/ui/UiRenderSystem.hpp"

namespace engine::ui {

/// Registers every interface component and system in a coordinator.
void registerInterface(core::Coordinator& coordinator)
{
	coordinator.registerComponent<UiTransform>();
	coordinator.registerComponent<UiRect>();
	coordinator.registerComponent<UiImage>();
	coordinator.registerComponent<UiText>();
	coordinator.registerComponent<UiButton>();
	coordinator.registerComponent<UiFocusable>();
	coordinator.registerComponent<UiButtonStyle>();
	coordinator.registerComponent<UiColorStyle>();
	coordinator.registerSystem<UiInputSystem>();
	coordinator.setSystemSignature<UiInputSystem>(UiInputSystem::signature(coordinator));
	coordinator.registerSystem<UiButtonStyleSystem>();
	coordinator.setSystemSignature<UiButtonStyleSystem>(UiButtonStyleSystem::signature(coordinator));
	coordinator.registerSystem<UiRenderSystem>();
	coordinator.setSystemSignature<UiRenderSystem>(UiRenderSystem::signature(coordinator));
}

/// Updates the buttons from the input, then shows their state. Clicks are published on the bus as UiClicked.
void updateInterface(core::Coordinator& coordinator, const input::IInputBackend& input, const render::IGraphicsBackend& backend, core::EventBus& bus)
{
	coordinator.getSystem<UiInputSystem>().update(coordinator, input, backend.screenSize(), bus);
	coordinator.getSystem<UiButtonStyleSystem>().update(coordinator);
}

/// Draws the interface in screen space, over what is already in the frame.
void renderInterface(core::Coordinator& coordinator, render::IGraphicsBackend& backend)
{
	coordinator.getSystem<UiRenderSystem>().render(coordinator, backend);
}

}
