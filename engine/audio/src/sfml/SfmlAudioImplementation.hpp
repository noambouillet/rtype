/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML objects owned by SfmlAudio, kept out of the public header
*/

#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "engine/audio/SfmlAudio.hpp"

namespace engine::audio {

struct SfmlAudio::Implementation {
	sf::SoundBuffer& loadBuffer(const std::string& name);

	std::filesystem::path root;
	sf::Music music;
	std::unordered_map<std::string, sf::SoundBuffer> buffers;
	std::unordered_set<std::string> missing;
	std::list<sf::Sound> playing;
};

}
