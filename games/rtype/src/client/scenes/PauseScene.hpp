/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Pause menu, drawn over the game
*/

#pragma once

#include <string_view>

#include "client/scenes/ClientContext.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/scene/Scene.hpp"
#include "engine/ui/UiEvents.hpp"

namespace rtype::client {

class PauseScene : public engine::core::Scene {
public:
	static constexpr std::string_view RESUME = "resume";
	static constexpr std::string_view MAIN_MENU = "main_menu";
	static constexpr std::string_view QUIT = "quit";
	static constexpr float TITLE_OFFSET = -140.0F;
	static constexpr engine::render::Color SHADE_COLOR{.r = 0, .g = 0, .b = 0, .a = 160};

	explicit PauseScene(ClientContext& context);

	void onEnter() override;
	void handleInput() override;
	void update(float deltaTime) override;
	void render() override;

private:
	void onClick(const engine::ui::UiClicked& click);

	ClientContext& _context;
	engine::core::Coordinator _coordinator;
	engine::core::EventBus _events;
	engine::core::Subscription _onClick;
	engine::core::Entity _shade;
};

}
