/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML client window implementation
*/

#include "Sfml.hpp"

#include "SpriteRegions.hpp"
#include "client/GameConfig.hpp"
#include "client/network/GameClient.hpp"

namespace rtype {

/// Opens the window and prepares the game using the AssetManager
Sfml::Sfml(AssetManager& assets)
	: _window(sf::VideoMode({client_config::WINDOW_WIDTH, client_config::WINDOW_HEIGHT}), "R-Type"),
	  _assets(assets),
	  _visualEffects(assets, _sounds),
	  _joystickId(0),
	  _lastFrame(std::chrono::steady_clock::now()),
	  _background(assets, _window),
	  _animationStart(std::chrono::steady_clock::now())
{
	_window.setFramerateLimit(60);
	_sounds.playMusic(MUSIC_FILE);
}

bool Sfml::isOpen() const
{
	return _window.isOpen();
}

/// Closes the window on close or Escape events.
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

/// Combines keyboard and joystick buttons into the protocol bit field.
std::uint8_t Sfml::buttons() const
{
	return keyboardButtons() | joystickButtons();
}

/// Updates the scrolling background based on the elapsed time since the last frame.
void Sfml::updateBackground()
{
	const auto now = std::chrono::steady_clock::now();
	const float elapsedTime = std::chrono::duration<float>(now - _lastFrame).count();
	_background.update(elapsedTime);
	_lastFrame = now;
}

/// Plays the local weapon effect before waiting for the server's projectile spawn.
void Sfml::predictLocalFire(std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId, const std::optional<network::WorldSnapshot>& snapshot)
{
	_visualEffects.predictLocalFire(buttons, localPlayerId, snapshot);
}

/// Reads the keyboard's current input state.
std::uint8_t Sfml::keyboardButtons() const
{
	if (!_window.hasFocus()) {
		return 0;
	}
	std::uint8_t value = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
		value |= network::BUTTON_UP;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
		value |= network::BUTTON_DOWN;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
		value |= network::BUTTON_LEFT;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
		value |= network::BUTTON_RIGHT;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
		value |= network::BUTTON_FIRE;
	}
	return value;
}

/// Reads the first joystick when it is connected.
std::uint8_t Sfml::joystickButtons() const
{
	if (!sf::Joystick::isConnected(_joystickId)) {
		return 0;
	}
	std::uint8_t value = 0;
	const float horizontal = sf::Joystick::getAxisPosition(_joystickId, sf::Joystick::Axis::X);
	const float vertical = sf::Joystick::getAxisPosition(_joystickId, sf::Joystick::Axis::Y);
	if (vertical < -JOYSTICK_THRESHOLD) {
		value |= network::BUTTON_UP;
	}
	if (vertical > JOYSTICK_THRESHOLD) {
		value |= network::BUTTON_DOWN;
	}
	if (horizontal < -JOYSTICK_THRESHOLD) {
		value |= network::BUTTON_LEFT;
	}
	if (horizontal > JOYSTICK_THRESHOLD) {
		value |= network::BUTTON_RIGHT;
	}
	if (sf::Joystick::isButtonPressed(_joystickId, 0)) {
		value |= network::BUTTON_FIRE;
	}
	return value;
}

/// Draws a player ship, tinted by its player ID.
void Sfml::drawPlayer(const network::WorldEntity& entity)
{
	std::uint8_t safeSpriteId = entity.spriteId;
	if (safeSpriteId >= sprites::PLAYER_SHIPS.size()) {
		safeSpriteId = 0;
	}
	sf::Sprite sprite = _assets.sprite(sprites::PLAYER_SHEET, sprites::PLAYER_SHIPS[safeSpriteId]);
	sprite.setOrigin({16.0F, 8.0F});
	sprite.setScale({SHIP_SCALE, SHIP_SCALE});
	sprite.setPosition({entity.x, entity.y});
	_window.draw(sprite);
}

/// Draws a projectile fired by a player.
void Sfml::drawPlayerShot(const network::WorldEntity& entity)
{
	const std::chrono::milliseconds age = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _animationStart);
	const std::size_t frameIndex = static_cast<std::size_t>(age.count() / SHOT_FRAME_TIME.count()) % sprites::PLAYER_SHOT_FRAMES.size();
	sf::Sprite sprite = _assets.sprite(sprites::SHOT_SHEET, sprites::PLAYER_SHOT_FRAMES[frameIndex]);
	sprite.setOrigin({8.0F, 6.0F});
	sprite.setScale({SHOT_SCALE, SHOT_SCALE});
	sprite.setPosition({entity.x, entity.y});
	_window.draw(sprite);
}

/// Draws a projectile fired by an enemy: a Scant beam or, by default, an aimed orb.
void Sfml::drawEnemyShot(const network::WorldEntity& entity)
{
	const bool beam = static_cast<network::ShotKind>(entity.spriteId) == network::ShotKind::Beam;
	sf::Sprite sprite = beam ? _assets.sprite(sprites::SCANT_SHEET, sprites::SCANT_BEAM)
							 : _assets.sprite(sprites::ENEMY_SHOT_SHEET, sprites::ENEMY_SHOT);
	sprite.setOrigin(beam ? sf::Vector2f{27.0F, 8.0F} : sf::Vector2f{3.5F, 3.0F});
	sprite.setScale({SHOT_SCALE, SHOT_SCALE});
	sprite.setPosition({entity.x, entity.y});
	_window.draw(sprite);
}

/// Draws an enemy: a Bug, a Scant, a Bink or, by default, a Patapata.
void Sfml::drawEnemy(const network::WorldEntity& entity)
{
	sf::Sprite sprite = _assets.sprite(sprites::ENEMY_SHEET, sprites::ENEMY_PATAPATA);
	sf::Vector2f origin{16.5F, 18.0F};

	switch (static_cast<network::EnemyKind>(entity.spriteId)) {
		case network::EnemyKind::Bug:
			sprite = _assets.sprite(sprites::BUG_SHEET, sprites::ENEMY_BUG);
			origin = {16.5F, 17.0F};
			break;
		case network::EnemyKind::Scant:
			sprite = _assets.sprite(sprites::SCANT_SHEET, sprites::ENEMY_SCANT);
			origin = {28.0F, 27.0F};
			break;
		case network::EnemyKind::Bink:
			sprite = _assets.sprite(sprites::BINK_SHEET, sprites::ENEMY_BINK);
			origin = {16.0F, 14.5F};
			break;
		default:
			break;
	}
	sprite.setOrigin(origin);
	sprite.setScale({SHIP_SCALE, SHIP_SCALE});
	sprite.setPosition({entity.x, entity.y});
	_window.draw(sprite);
}

/// Draws the latest snapshot and temporary sprite animations in one frame.
void Sfml::displayWindow(const std::optional<network::WorldSnapshot>& snapshot, const std::vector<network::WorldEventNotice>& events)
{
	_window.clear(sf::Color::Black);
	_background.draw(_window);
	if (snapshot) {
		for (const auto& entity : snapshot->entities) {
			switch (entity.entityType) {
				case network::EntityType::Player:
					drawPlayer(entity);
					break;
				case network::EntityType::PlayerProjectile:
					drawPlayerShot(entity);
					break;
				case network::EntityType::Enemy:
					drawEnemy(entity);
					break;
				case network::EntityType::EnemyProjectile:
					drawEnemyShot(entity);
					break;
				default:
					break;
			}
		}
	}
	_visualEffects.display(_window, snapshot, events);
	_window.display();
}

}
