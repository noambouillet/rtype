/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML graphics backend
*/

#include "Sfml.hpp"

#include <cmath>
#include <map>
#include <string>

#include "client/GameConfig.hpp"
#include "client/view/SpriteRegions.hpp"
#include "game/GameTypes.hpp"
#include "game_engine/ConfigParser.hpp"
#include "utils/RTypeError.hpp"

namespace rtype {

/// Initializes the SFML window, loads client settings, and prepares the keyboard button mapping.
Sfml::Sfml(AssetManager& assets)
	: _window(sf::VideoMode({client_config::WINDOW_WIDTH, client_config::WINDOW_HEIGHT}), "R-Type"),
	  _assets(assets),
	  _joystickId(0),
	  _autoFireEnabled(false),
	  _autoFireKeyWasPressed(false)
{
	const std::string configFile = std::string(client_config::SETTINGS_FILE);
	std::map<std::string, std::string> config = engine::ConfigParser::parseFile(configFile);
	_engineSettings.loadFromMap(config);
	_bindings.loadFromMap(config);
	if (config.empty()) {
		_engineSettings.saveToMap(config);
		_bindings.saveToMap(config);
		if (!engine::ConfigParser::saveFile(configFile, config)) {
			throw ConfigError("Unable to create settings file: " + configFile);
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
std::uint8_t Sfml::joystickButtons() const
{
	if (!sf::Joystick::isConnected(_joystickId)) {
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
	if (_bindings.joystickFire() < sf::Joystick::getButtonCount(_joystickId) && sf::Joystick::isButtonPressed(_joystickId, _bindings.joystickFire())) {
		value |= game::BUTTON_FIRE;
	}
	return value;
}

void Sfml::beginFrame()
{
	_window.clear(sf::Color::Black);
}

/// Draws the repeated background at the offset selected by the render system.
void Sfml::drawBackground(std::string_view texture, float offset)
{
	_assets.setRepeated(texture, true);
	const sf::Texture& image = _assets.texture(texture);
	const sf::Vector2u size = _window.getSize();
	if (size.x == 0 || size.y == 0 || image.getSize().x == 0) {
		return;
	}
	const float wrapped = std::fmod(offset, static_cast<float>(image.getSize().x));
	const int left = static_cast<int>(wrapped);
	sf::Sprite sprite(image, sf::IntRect({left, 0}, {static_cast<int>(size.x) + 1, static_cast<int>(size.y)}));
	sprite.setPosition({-(wrapped - static_cast<float>(left)), 0.0F});
	_window.draw(sprite);
}

/// Translates a library independent sprite command into an SFML draw call.
void Sfml::drawSprite(const render::SpriteDraw& command)
{
	if (!_engineSettings.screenFlashEnabled() && command.texture == sprites::EXPLOSION_SHEET) {
		return;
	}
	const render::Rect& region = command.region;
	sf::Sprite sprite = _assets.sprite(command.texture, sf::IntRect({region.position.x, region.position.y}, {region.size.x, region.size.y}));
	sprite.setOrigin({command.originX, command.originY});
	sprite.setScale({command.scale, command.scale});
	sprite.setRotation(sf::degrees(command.rotation));
	sprite.setPosition({command.x, command.y});
	_window.draw(sprite);
}

void Sfml::endFrame()
{
	_window.display();
}

}
