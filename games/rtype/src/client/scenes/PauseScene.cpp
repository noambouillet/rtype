/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Pause menu, drawn over the game
*/

#include "PauseScene.hpp"

#include <memory>
#include <string>

#include "client/app/ClientControls.hpp"
#include "client/scenes/MainMenuScene.hpp"
#include "client/scenes/MenuWidgets.hpp"
#include "engine/ui/Interface.hpp"
#include "engine/ui/UiInputSystem.hpp"

namespace rtype::client {

PauseScene::PauseScene(ClientContext& context) : _context(context), _coordinator(), _events(), _onClick(), _shade()
{
}

/// Builds a dark layer over the game, the title and the buttons.
void PauseScene::onEnter()
{
	engine::ui::registerInterface(_coordinator);
	_coordinator.getSystem<engine::ui::UiInputSystem>().setNavigation(std::string(ClientControls::UI_UP), std::string(ClientControls::UI_DOWN), std::string(ClientControls::UI_CONFIRM));
	_shade = _coordinator.createEntity();
	_coordinator.addComponent(_shade, engine::ui::UiTransform{.anchor = engine::ui::Anchor::Center, .offsetX = 0.0F, .offsetY = 0.0F, .width = 0.0F, .height = 0.0F, .layer = 0});
	_coordinator.addComponent(_shade, engine::ui::UiRect{.color = SHADE_COLOR});
	MenuWidgets::makeLabel(_coordinator, _context.font, {.text = "Paused", .size = MenuWidgets::TITLE_TEXT_SIZE, .offsetY = TITLE_OFFSET, .color = MenuWidgets::TEXT_COLOR});
	MenuWidgets::makeButton(_coordinator, _context.font, {.label = "Resume", .action = std::string(RESUME), .offsetY = 0.0F, .focusOrder = 0});
	MenuWidgets::makeButton(_coordinator, _context.font, {.label = "Main menu", .action = std::string(MAIN_MENU), .offsetY = MenuWidgets::BUTTON_SPACING, .focusOrder = 1});
	MenuWidgets::makeButton(_coordinator, _context.font, {.label = "Quit", .action = std::string(QUIT), .offsetY = MenuWidgets::BUTTON_SPACING * 2.0F, .focusOrder = 2});
	_onClick = _events.subscribe<engine::ui::UiClicked>([this](const engine::ui::UiClicked& click) { onClick(click); });
}

/// The back action closes the menu, like the Resume button.
void PauseScene::handleInput()
{
	engine::ui::updateInterface(_coordinator, _context.input, _context.graphics, _events);
	if (_context.input.justPressed(ClientControls::UI_BACK)) {
		_context.scenes.pop();
	}
}

/// Keeps the dark layer as large as the window, which may have been resized.
void PauseScene::update([[maybe_unused]] float deltaTime)
{
	const engine::render::Point screen = _context.graphics.screenSize();
	auto& transform = _coordinator.getComponent<engine::ui::UiTransform>(_shade);
	transform.width = static_cast<float>(screen.x);
	transform.height = static_cast<float>(screen.y);
}

void PauseScene::render()
{
	engine::ui::renderInterface(_coordinator, _context.graphics);
}

/// Main menu leaves the game, which disconnects from the server. Quit empties the scene stack, which closes the client.
void PauseScene::onClick(const engine::ui::UiClicked& click)
{
	if (click.action == RESUME) {
		_context.scenes.pop();
	}
	if (click.action == MAIN_MENU) {
		_context.scenes.clear();
		_context.scenes.push(std::make_unique<MainMenuScene>(_context));
	}
	if (click.action == QUIT) {
		_context.scenes.clear();
	}
}

}
