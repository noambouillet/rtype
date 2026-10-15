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

#include "AssetManager.hpp"
#include "game_engine/Graphics/GraphicsBackend.hpp"

namespace rtype {

class Sfml : public render::IGraphicsBackend {
public:
	explicit Sfml(AssetManager& assets);
	bool isOpen() const;
	void handleEvent();
	std::uint8_t buttons() const;
	void beginFrame() override;
	void drawBackground(std::string_view texture, float offset) override;
	void drawSprite(const render::SpriteDraw& command) override;
	void endFrame() override;

private:
	static constexpr float JOYSTICK_THRESHOLD = 50.0F;

	std::uint8_t keyboardButtons() const;
	std::uint8_t joystickButtons() const;

	sf::RenderWindow _window;
	AssetManager& _assets;
	unsigned int _joystickId;
};

}
