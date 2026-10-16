/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML graphics backend
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "AssetManager.hpp"
#include "KeyBindings.hpp"
#include "game_engine/EngineSettings.hpp"
#include "game_engine/Graphics/GraphicsBackend.hpp"

namespace rtype {

class Sfml : public render::IGraphicsBackend {
public:
	explicit Sfml(AssetManager& assets);
	bool isOpen() const;
	void handleEvent();
	std::uint8_t buttons();
	void beginFrame() override;
	void drawBackground(std::string_view texture, float offset) override;
	void drawSprite(const render::SpriteDraw& command) override;
	void endFrame() override;

private:
	std::uint8_t keyboardButtons();
	std::uint8_t joystickButtons();

	sf::RenderWindow _window;
	AssetManager& _assets;
	engine::EngineSettings _engineSettings;
	KeyBindings _bindings;
	std::vector<std::pair<sf::Keyboard::Key, ActionType>> _keyboardButtons;
	std::uint32_t _joystickId;
	bool _autoFireEnabled;
	bool _autoFireKeyWasPressed;
	bool _joystickAutoFireWasPressed;
};

}
