/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML window wrapper and input driver
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "client/SFML/KeyBindings.hpp"
#include "engine/core/EngineSettings.hpp"
#include "engine/render/AssetManager.hpp"
#include "engine/render/GraphicsBackend.hpp"

namespace rtype {

class Sfml : public engine::render::IGraphicsBackend {
public:
	explicit Sfml(engine::render::AssetManager& assets);

	void handleEvent();
	[[nodiscard]] bool isOpen() const;
	[[nodiscard]] std::uint8_t buttons();
	engine::render::TextureId loadTextureFromFile(const std::string& filepath) override;
	void beginFrame() override;
	void drawBackground(std::string_view texture, float offset) override;
	void drawSprite(const engine::render::SpriteDraw& command) override;
	void endFrame() override;

private:
	std::unordered_map<engine::render::TextureId, sf::Texture>::iterator resolveTexture(
		engine::render::TextureId textureId);

	std::uint8_t keyboardButtons();
	std::uint8_t joystickButtons();

	sf::RenderWindow _window;
	engine::render::AssetManager& _assets;
	std::unordered_map<engine::render::TextureId, sf::Texture> _loadedTextures;
	std::unordered_map<std::string, engine::render::TextureId> _pathToTextureId;
	engine::render::TextureId _nextTextureId;
	engine::render::TextureId _fallbackTextureId;
	engine::core::EngineSettings _engineSettings;
	KeyBindings _bindings;
	std::vector<std::pair<sf::Keyboard::Key, ActionType>> _keyboardButtons;
	std::uint32_t _joystickId;
	bool _autoFireEnabled;
	bool _autoFireKeyWasPressed;
	bool _joystickAutoFireWasPressed;
};

}
