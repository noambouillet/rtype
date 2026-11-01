/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Title screen
*/

#include "MainMenuScene.hpp"

#include <memory>
#include <utility>

#include "client/app/ClientControls.hpp"
#include "client/scenes/GameScene.hpp"
#include "client/scenes/MenuWidgets.hpp"
#include "engine/core/ecs/Components.hpp"
#include "engine/net/NetworkError.hpp"
#include "engine/render/Components.hpp"
#include "engine/render/RenderSystem.hpp"
#include "engine/ui/Interface.hpp"
#include "engine/ui/UiInputSystem.hpp"

namespace rtype::client {

/// The message is shown under the buttons, for example to tell why the last game ended.
MainMenuScene::MainMenuScene(ClientContext& context, std::string message)
	: _context(context), _message(std::move(message)), _coordinator(), _events(), _onClick(), _messageLabel(), _deltaTime(0.0F)
{
}

/// Builds the scrolling background, the title and the buttons.
void MainMenuScene::onEnter()
{
	_coordinator.registerComponent<engine::core::Position>();
	_coordinator.registerComponent<engine::render::Sprite>();
	auto& background = _coordinator.registerSystem<engine::render::RenderSystem>();
	_coordinator.setSystemSignature<engine::render::RenderSystem>(engine::render::RenderSystem::signature(_coordinator));
	background.setBackground(_context.assets.getTexture("background"), BACKGROUND_SPEED);
	engine::ui::registerInterface(_coordinator);
	_coordinator.getSystem<engine::ui::UiInputSystem>().setNavigation(std::string(ClientControls::UI_UP), std::string(ClientControls::UI_DOWN), std::string(ClientControls::UI_CONFIRM));
	MenuWidgets::makeLabel(_coordinator, _context.font, {.text = "R-TYPE", .size = MenuWidgets::TITLE_TEXT_SIZE, .offsetY = TITLE_OFFSET, .color = MenuWidgets::TEXT_COLOR});
	MenuWidgets::makeButton(_coordinator, _context.font, {.label = "Play", .action = std::string(PLAY), .offsetY = 0.0F, .focusOrder = 0});
	MenuWidgets::makeButton(_coordinator, _context.font, {.label = "Quit", .action = std::string(QUIT), .offsetY = MenuWidgets::BUTTON_SPACING, .focusOrder = 1});
	_messageLabel = MenuWidgets::makeLabel(_coordinator, _context.font, {.text = _message, .size = MenuWidgets::MESSAGE_TEXT_SIZE, .offsetY = MenuWidgets::BUTTON_SPACING * 2.0F, .color = MenuWidgets::MESSAGE_COLOR});
	_onClick = _events.subscribe<engine::ui::UiClicked>([this](const engine::ui::UiClicked& click) { onClick(click); });
}

/// Updates the buttons from the mouse, keyboard and joystick; their clicks reach onClick.
void MainMenuScene::handleInput()
{
	engine::ui::updateInterface(_coordinator, _context.input, _context.graphics, _events);
}

void MainMenuScene::update(float deltaTime)
{
	_deltaTime = deltaTime;
}

void MainMenuScene::render()
{
	_coordinator.getSystem<engine::render::RenderSystem>().render(_coordinator, _context.graphics, _deltaTime);
	engine::ui::renderInterface(_coordinator, _context.graphics);
}

/// Play starts a game in place of the menu, Quit empties the scene stack, which closes the client.
void MainMenuScene::onClick(const engine::ui::UiClicked& click)
{
	if (click.action == PLAY) {
		try {
			_context.scenes.replace(std::make_unique<GameScene>(_context));
		} catch (const engine::net::NetworkError& error) {
			_context.logger.error(error.what());
			_coordinator.getComponent<engine::ui::UiText>(_messageLabel).text = "Cannot reach the server";
		}
	}
	if (click.action == QUIT) {
		_context.scenes.clear();
	}
}

}
