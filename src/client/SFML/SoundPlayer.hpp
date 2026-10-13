/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML audio playback
*/

#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace rtype {

class SoundPlayer {
public:
	SoundPlayer();
	void playMusic(std::string_view name);
	void playSound(std::string_view name, float volume);
	void stopMusic();

private:
	static constexpr float MUSIC_VOLUME = 20.0F;

	sf::SoundBuffer& loadBuffer(const std::string& name);

	std::filesystem::path _root;
	sf::Music _music;
	std::unordered_map<std::string, sf::SoundBuffer> _buffers;
	std::unordered_set<std::string> _missing;
	std::list<sf::Sound> _playing;
};

}
