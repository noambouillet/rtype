/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML graphics backend
*/

#include "Sfml.hpp"

#include <cmath>

#include "client/GameConfig.hpp"
#include "game/GameTypes.hpp"

namespace rtype {

Sfml::Sfml(AssetManager& assets)
	: _window(sf::VideoMode({client_config::WINDOW_WIDTH, client_config::WINDOW_HEIGHT}), "R-Type"),
	  _assets(assets),
	  _joystickId(0)
{
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

std::uint8_t Sfml::buttons() const
{
	return keyboardButtons() | joystickButtons();
}

std::uint8_t Sfml::keyboardButtons() const
{
	if (!_window.hasFocus()) {
		return 0;
	}
	std::uint8_t value = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
		value |= game::BUTTON_UP;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
		value |= game::BUTTON_DOWN;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
		value |= game::BUTTON_LEFT;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
		value |= game::BUTTON_RIGHT;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
		value |= game::BUTTON_FIRE;
	}
	return value;
}

std::uint8_t Sfml::joystickButtons() const
{
	if (!sf::Joystick::isConnected(_joystickId)) {
		return 0;
	}
	std::uint8_t value = 0;
	const float horizontal = sf::Joystick::getAxisPosition(_joystickId, sf::Joystick::Axis::X);
	const float vertical = sf::Joystick::getAxisPosition(_joystickId, sf::Joystick::Axis::Y);
	if (vertical < -JOYSTICK_THRESHOLD) {
		value |= game::BUTTON_UP;
	}
	if (vertical > JOYSTICK_THRESHOLD) {
		value |= game::BUTTON_DOWN;
	}
	if (horizontal < -JOYSTICK_THRESHOLD) {
		value |= game::BUTTON_LEFT;
	}
	if (horizontal > JOYSTICK_THRESHOLD) {
		value |= game::BUTTON_RIGHT;
	}
	if (sf::Joystick::isButtonPressed(_joystickId, 0)) {
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
