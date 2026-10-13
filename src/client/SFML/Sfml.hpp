/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sfml
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "AssetManager.hpp"
#include "ScrollingBackground.hpp"
#include "SoundPlayer.hpp"
#include "VisualEffects.hpp"

namespace rtype {
namespace network {
struct WorldEntity;
struct WorldEventNotice;
struct WorldSnapshot;
}

class Sfml {
private:
	static constexpr float JOYSTICK_THRESHOLD = 50.0F;
	static constexpr float PLAYER_SPEED = 250;
	static constexpr float MAX_FRAME_SECONDS = 0.1;
	static constexpr std::uint32_t WINDOW_WIDTH = 1280;
	static constexpr std::uint32_t WINDOW_HEIGHT = 720;
	static constexpr float SHIP_SCALE = 3.0F;  // player ships and enemies; the server colliders match this scale
	static constexpr float SHOT_SCALE = 2.0F;
	static constexpr auto SHOT_FRAME_TIME = std::chrono::milliseconds(80);
	static constexpr std::string_view MUSIC_FILE = "music.wav";

	std::uint8_t keyboardButtons() const;
	std::uint8_t joystickButtons() const;
	void drawPlayer(const network::WorldEntity& entity);
	void drawPlayerShot(const network::WorldEntity& entity);
	void drawEnemy(const network::WorldEntity& entity);
	void drawEnemyShot(const network::WorldEntity& entity);

	sf::RenderWindow _window;
	AssetManager& _assets;
	SoundPlayer _sounds;
	VisualEffects _visualEffects;
	unsigned int _joystickId;
	std::chrono::steady_clock::time_point _lastFrame;
	ScrollingBackground _background;
	std::chrono::steady_clock::time_point _animationStart;

public:
	explicit Sfml(AssetManager& assets);
	bool isOpen() const;
	void handleEvent();
	std::uint8_t buttons() const;
	void updateBackground();
	void predictLocalFire(std::uint8_t buttons, std::optional<std::uint8_t> localPlayerId, const std::optional<network::WorldSnapshot>& snapshot);
	void displayWindow(const std::optional<network::WorldSnapshot>& snapshot, const std::vector<network::WorldEventNotice>& events);
};

}
