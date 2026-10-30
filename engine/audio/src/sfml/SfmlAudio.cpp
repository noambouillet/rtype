/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML audio backend
*/

#include "engine/audio/SfmlAudio.hpp"

#include <string>
#include <utility>

#include "SfmlAudioImplementation.hpp"
#include "engine/audio/AudioError.hpp"

namespace engine::audio {

/// Music and sound names given to this backend are file paths relative to root (or direct paths if root is empty).
SfmlAudio::SfmlAudio(std::filesystem::path root) : _implementation(std::make_unique<Implementation>())
{
	_implementation->root = std::move(root);
}

SfmlAudio::~SfmlAudio() = default;

/// Plays music in a loop, or stays silent if the file cannot be opened.
void SfmlAudio::playMusic(std::string_view name, float volume)
{
	const std::filesystem::path musicPath = _implementation->root.empty()
												? std::filesystem::path(name)
												: _implementation->root / std::filesystem::path(name).lexically_normal();
	if (!_implementation->music.openFromFile(musicPath)) {
		return;
	}
	_implementation->music.setLooping(true);
	_implementation->music.setVolume(volume);
	_implementation->music.play();
}

void SfmlAudio::stopMusic()
{
	_implementation->music.stop();
}

/// Loads "name" into the cache or throws if the file cannot be read.
sf::SoundBuffer& SfmlAudio::Implementation::loadBuffer(const std::string& name)
{
	const auto [bufferEntry, wasInserted] = buffers.try_emplace(name);
	if (wasInserted) {
		const std::filesystem::path soundPath = root.empty()
													? std::filesystem::path(name)
													: root / std::filesystem::path(name).lexically_normal();
		if (!bufferEntry->second.loadFromFile(soundPath)) {
			buffers.erase(bufferEntry);
			throw AudioError("Cannot load sound: " + soundPath.string());
		}
	}
	return bufferEntry->second;
}

/// Plays a cached sound once, or nothing if its file is missing.
void SfmlAudio::playSound(std::string_view name, float volume)
{
	const std::string key(name);
	if (_implementation->missing.contains(key)) {
		return;
	}
	std::erase_if(_implementation->playing, [](const sf::Sound& sound) { return sound.getStatus() == sf::Sound::Status::Stopped; });
	try {
		sf::Sound& sound = _implementation->playing.emplace_back(_implementation->loadBuffer(key));
		sound.setVolume(volume);
		sound.play();
	} catch (const AudioError&) {
		_implementation->missing.insert(key);
	}
}

}
