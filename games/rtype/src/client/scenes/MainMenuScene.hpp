/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Title screen
*/

#pragma once

#include <string>
#include <string_view>

#include "client/scenes/ClientContext.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/scene/Scene.hpp"
#include "engine/ui/UiEvents.hpp"

namespace rtype::client {

class MainMenuScene : public engine::core::Scene {
public:
	static constexpr std::string_view PLAY = "play";
	static constexpr std::string_view QUIT = "quit";
	static constexpr float BACKGROUND_SPEED = 20.0F;
	static constexpr float TITLE_OFFSET = -180.0F;

	explicit MainMenuScene(ClientContext& context, std::string message = "");

	void onEnter() override;
	void handleInput() override;
	void update(float deltaTime) override;
	void render() override;

private:
	void onClick(const engine::ui::UiClicked& click);

	ClientContext& _context;
	std::string _message;
	engine::core::Coordinator _coordinator;
	engine::core::EventBus _events;
	engine::core::Subscription _onClick;
	engine::core::Entity _messageLabel;
	float _deltaTime;
};

}
