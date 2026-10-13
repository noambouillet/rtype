/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SFML audio playback
*/

#include "SoundPlayer.hpp"

#include <string>

#include "utils/RTypeError.hpp"

namespace rtype {

SoundPlayer::SoundPlayer() : _root("assets/sounds")
{
}

/// Plays music in a loop, or keeps the game silent if the file cannot be opened.
void SoundPlayer::playMusic(std::string_view name)
{
	const std::filesystem::path musicPath = _root / std::filesystem::path(name).lexically_normal();
	if (!_music.openFromFile(musicPath)) {
		return;
	}
	_music.setLooping(true);
	_music.setVolume(MUSIC_VOLUME);
	_music.play();
}

void SoundPlayer::stopMusic()
{
	_music.stop();
}

/// Loads "name" into the cache or throws if the file cannot be read.
sf::SoundBuffer& SoundPlayer::loadBuffer(const std::string& name)
{
	const auto [it, inserted] = _buffers.try_emplace(name);
	if (inserted) {
		const std::filesystem::path soundPath = _root / std::filesystem::path(name).lexically_normal();
		if (!it->second.loadFromFile(soundPath)) {
			_buffers.erase(it);
			throw AudioError("Cannot load sound: " + soundPath.string());
		}
	}
	return it->second;
}

/// Plays a cached sound once, or nothing if its file is missing.
void SoundPlayer::playSound(std::string_view name, float volume)
{
	const std::string key(name);
	if (_missing.contains(key)) {
		return;
	}
	std::erase_if(_playing, [](const sf::Sound& sound) { return sound.getStatus() == sf::Sound::Status::Stopped; });
	try {
		sf::Sound& sound = _playing.emplace_back(loadBuffer(key));
		sound.setVolume(volume);
		sound.play();
	} catch (const AudioError&) {
		_missing.insert(key);
	}
}

}
