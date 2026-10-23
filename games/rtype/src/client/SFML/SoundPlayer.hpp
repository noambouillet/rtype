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

#include "engine/audio/AudioBackend.hpp"

namespace rtype {

class SoundPlayer : public engine::audio::IAudioBackend {
public:
	SoundPlayer();
	void playMusic(std::string_view name, float volume) override;
	void stopMusic() override;
	void playSound(std::string_view name, float volume) override;

private:
	sf::SoundBuffer& loadBuffer(const std::string& name);

	std::filesystem::path _root;
	sf::Music _music;
	std::unordered_map<std::string, sf::SoundBuffer> _buffers;
	std::unordered_set<std::string> _missing;
	std::list<sf::Sound> _playing;
};

}
