/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML graphics backend
*/

#include "Sfml.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <tuple>

#include "client/GameConfig.hpp"
#include "client/view/SpriteRegions.hpp"
#include "engine/core/ConfigParser.hpp"
#include "engine/core/Error.hpp"
#include "engine/render/AssetManager.hpp"
#include "game/GameTypes.hpp"

namespace rtype {

/// Initializes the SFML window, loads client settings, and prepares the keyboard button mapping.
Sfml::Sfml(engine::render::AssetManager& assets)
	: _window(sf::VideoMode({client_config::WINDOW_WIDTH, client_config::WINDOW_HEIGHT}), "R-Type"),
	  _assets(assets),
	  _loadedTextures{},
	  _pathToTextureId{},
	  _nextTextureId(1),
	  _fallbackTextureId(0),
	  _joystickId(0),
	  _autoFireEnabled(false),
	  _autoFireKeyWasPressed(false),
	  _joystickAutoFireWasPressed(false)
{
	const std::string configFile = std::string(client_config::SETTINGS_FILE);
	std::map<std::string, std::string> config = engine::core::ConfigParser::parseFile(configFile);
	_engineSettings.loadFromMap(config);
	_bindings.loadFromMap(config);
	if (config.empty()) {
		_engineSettings.saveToMap(config);
		_bindings.saveToMap(config);
		if (!engine::core::ConfigParser::saveFile(configFile, config)) {
			throw engine::core::ConfigError("Unable to create settings file: " + configFile);
		}
	}

	const std::vector<ActionType> allActions = {
		ActionType::Up, ActionType::Down, ActionType::Left,
		ActionType::Right, ActionType::Fire, ActionType::AutoFire};
	_keyboardButtons.clear();
	_keyboardButtons.reserve(allActions.size());
	for (ActionType action : allActions) {
		const sf::Keyboard::Key key = _bindings.getBinding(action);
		_keyboardButtons.push_back({key, action});
	}
	_window.setFramerateLimit(60);
}

/// Checks whether the underlying SFML render window is open.
bool Sfml::isOpen() const
{
	return _window.isOpen();
}

/// Closes the window when its close button or Escape is pressed.
void Sfml::handleEvent()
{
	while (const auto event = _window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			_window.close();
		}
		if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
			if (key->code == sf::Keyboard::Key::Escape) {
				_window.close();
			}
		}
	}
}

/// Returns a bitmask of the currently pressed buttons, combining keyboard and joystick input.
std::uint8_t Sfml::buttons()
{
	return keyboardButtons() | joystickButtons();
}

/// Returns a bitmask of the currently pressed keyboard buttons, based on the configured key bindings.
std::uint8_t Sfml::keyboardButtons()
{
	if (!_window.hasFocus()) {
		_autoFireKeyWasPressed = false;
		return 0;
	}
	std::uint8_t value = 0;
	for (const auto& [key, action] : _keyboardButtons) {
		if (key == sf::Keyboard::Key::Unknown) {
			continue;
		}
		const bool pressed = sf::Keyboard::isKeyPressed(key);
		if (action == ActionType::AutoFire) {
			if (pressed && !_autoFireKeyWasPressed) {
				_autoFireEnabled = !_autoFireEnabled;
			}
			_autoFireKeyWasPressed = pressed;
			continue;
		}
		if (!pressed) {
			continue;
		}
		switch (action) {
			case ActionType::Up:
				value |= game::BUTTON_UP;
				break;
			case ActionType::Down:
				value |= game::BUTTON_DOWN;
				break;
			case ActionType::Left:
				value |= game::BUTTON_LEFT;
				break;
			case ActionType::Right:
				value |= game::BUTTON_RIGHT;
				break;
			case ActionType::Fire:
				value |= game::BUTTON_FIRE;
				break;
			case ActionType::AutoFire:
				break;
		}
	}
	if (_autoFireEnabled) {
		value |= game::BUTTON_FIRE;
	}
	return value;
}

/// Returns a bitmask of the currently pressed joystick buttons, based on the configured deadzone and fire button.
std::uint8_t Sfml::joystickButtons()
{
	if (!sf::Joystick::isConnected(_joystickId)) {
		_joystickAutoFireWasPressed = false;
		return 0;
	}
	std::uint8_t value = 0;
	const float horizontal = sf::Joystick::getAxisPosition(_joystickId, sf::Joystick::Axis::X);
	const float vertical = sf::Joystick::getAxisPosition(_joystickId, sf::Joystick::Axis::Y);
	const float threshold = static_cast<float>(_bindings.deadzone());
	if (vertical < -threshold) {
		value |= game::BUTTON_UP;
	}
	if (vertical > threshold) {
		value |= game::BUTTON_DOWN;
	}
	if (horizontal < -threshold) {
		value |= game::BUTTON_LEFT;
	}
	if (horizontal > threshold) {
		value |= game::BUTTON_RIGHT;
	}
	const unsigned int buttonCount = sf::Joystick::getButtonCount(_joystickId);
	if (_bindings.joystickFire() < buttonCount && sf::Joystick::isButtonPressed(_joystickId, _bindings.joystickFire())) {
		value |= game::BUTTON_FIRE;
	}
	const bool autoFirePressed = _bindings.joystickAutoFire() < buttonCount && sf::Joystick::isButtonPressed(_joystickId, _bindings.joystickAutoFire());
	if (autoFirePressed && !_joystickAutoFireWasPressed) {
		_autoFireEnabled = !_autoFireEnabled;
	}
	_joystickAutoFireWasPressed = autoFirePressed;
	if (_autoFireEnabled) {
		value |= game::BUTTON_FIRE;
	}
	return value;
}

/// Resolves a texture iterator by identifier with fallback support.
std::unordered_map<engine::render::TextureId, sf::Texture>::iterator Sfml::resolveTexture(
	engine::render::TextureId textureId)
{
	auto textureIterator = _loadedTextures.find(textureId);
	if (textureIterator != _loadedTextures.end()) {
		return textureIterator;
	}
	textureIterator = _loadedTextures.find(_fallbackTextureId);
	if (textureIterator != _loadedTextures.end()) {
		return textureIterator;
	}
	return _loadedTextures.begin();
}

/// Loads a texture from file into the SFML texture cache and returns an ID.
engine::render::TextureId Sfml::loadTextureFromFile(const std::string& filepath)
{
	const auto existingMapping = _pathToTextureId.find(filepath);
	if (existingMapping != _pathToTextureId.end()) {
		std::ignore = _loadedTextures[existingMapping->second].loadFromFile(std::filesystem::path(filepath));
		return existingMapping->second;
	}

	sf::Texture loadedTexture;
	if (!loadedTexture.loadFromFile(std::filesystem::path(filepath))) {
		std::cerr << "Sfml error: Failed to load texture: " << filepath << "\n";
		return 0;
	}
	const engine::render::TextureId assignedTextureId = _nextTextureId++;
	_loadedTextures.emplace(assignedTextureId, std::move(loadedTexture));
	_pathToTextureId.emplace(filepath, assignedTextureId);
	if (_fallbackTextureId == 0 && filepath.find("magenta-checkerboard") != std::string::npos) {
		_fallbackTextureId = assignedTextureId;
	}
	return assignedTextureId;
}

/// Clears the window for the upcoming frame.
void Sfml::beginFrame()
{
	_window.clear(sf::Color::Black);
}

/// Draws the repeated background at the offset selected by the render system.
void Sfml::drawBackground(std::string_view texture, float offset)
{
	const engine::render::TextureId textureId = _assets.getTexture(std::string(texture));
	const auto textureIterator = resolveTexture(textureId);
	if (textureIterator == _loadedTextures.end()) {
		return;
	}
	textureIterator->second.setRepeated(true);
	const sf::Texture& backgroundImage = textureIterator->second;
	const sf::Vector2u windowSize = _window.getSize();
	if (windowSize.x == 0 || windowSize.y == 0 || backgroundImage.getSize().x == 0) {
		return;
	}
	const float wrappedOffset = std::fmod(offset, static_cast<float>(backgroundImage.getSize().x));
	const int leftPosition = static_cast<int>(wrappedOffset);
	sf::Sprite backgroundSprite(backgroundImage,
		sf::IntRect({leftPosition, 0}, {static_cast<int>(windowSize.x) + 1, static_cast<int>(windowSize.y)}));
	backgroundSprite.setPosition({-(wrappedOffset - static_cast<float>(leftPosition)), 0.0F});
	_window.draw(backgroundSprite);
}

/// Translates a library independent sprite command into an SFML draw call.
void Sfml::drawSprite(const engine::render::SpriteDraw& command)
{
	if (!_engineSettings.screenFlashEnabled() && command.texture == sprites::EXPLOSION_SHEET) {
		return;
	}
	const engine::render::TextureId textureId = _assets.getTexture(std::string(command.texture));
	const auto textureIterator = resolveTexture(textureId);
	if (textureIterator == _loadedTextures.end()) {
		return;
	}
	const sf::Texture& spriteTexture = textureIterator->second;
	const engine::render::Rect& region = command.region;
	sf::Sprite renderedSprite(spriteTexture,
		sf::IntRect({region.position.x, region.position.y}, {region.size.x, region.size.y}));
	renderedSprite.setOrigin({command.originX, command.originY});
	renderedSprite.setScale({command.scale, command.scale});
	renderedSprite.setRotation(sf::degrees(command.rotation));
	renderedSprite.setPosition({command.x, command.y});
	_window.draw(renderedSprite);
}

/// Displays the rendered frame onto the SFML window.
void Sfml::endFrame()
{
	_window.display();
}

}
