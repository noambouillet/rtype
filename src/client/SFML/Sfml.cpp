/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML graphics backend
*/

#include "Sfml.hpp"

#include <cmath>
#include <fstream>
#include <string>

#include "client/GameConfig.hpp"
#include "game/GameTypes.hpp"

namespace rtype {

/// Initializes the SFML window, loads the key bindings configuration, and prepares the keyboard button mapping.
Sfml::Sfml(AssetManager& assets)
	: _window(sf::VideoMode({client_config::WINDOW_WIDTH, client_config::WINDOW_HEIGHT}), "R-Type"),
	  _assets(assets),
	  _joystickId(0)
{
	const std::string configFile = std::string(client_config::BINDINGS_FILE);
	std::ifstream checkFile(configFile);
	if (!checkFile.is_open()) {
		_bindings.save(configFile);
	}
	checkFile.close();
	_bindings.load(configFile);

	const std::array<ActionType, key_bindings_config::ACTION_COUNT> allActions = {
		ActionType::Up, ActionType::Down, ActionType::Left,
		ActionType::Right, ActionType::Fire};
	_keyboardButtons.clear();
	for (ActionType action : allActions) {
		_keyboardButtons.push_back({_bindings.getBinding(action), action});
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
std::uint8_t Sfml::buttons() const
{
	return keyboardButtons() | joystickButtons();
}

/// Returns a bitmask of the currently pressed keyboard buttons, based on the configured key bindings.
std::uint8_t Sfml::keyboardButtons() const
{
	if (!_window.hasFocus()) {
		return 0;
	}
	std::uint8_t value = 0;
	for (const auto& [key, action] : _keyboardButtons) {
		if (!sf::Keyboard::isKeyPressed(key)) {
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
		}
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
