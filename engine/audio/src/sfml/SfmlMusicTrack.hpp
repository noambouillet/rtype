/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** One SFML music stream that can fade in and out
*/

#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>

#include "engine/audio/AudioBackend.hpp"
#include "engine/audio/Mixer.hpp"

namespace engine::audio {

class SfmlMusicTrack {
public:
	SfmlMusicTrack();

	[[nodiscard]] bool isPlaying() const;
	[[nodiscard]] bool isPlaying(const std::filesystem::path& path) const;
	bool open(const std::filesystem::path& path);
	void start(float volume, Seconds fade, const Mixer& mixer);
	void setVolume(float volume, const Mixer& mixer);
	void fadeTo(float target, Seconds fade);
	void update(float deltaTime, const Mixer& mixer);
	void applyVolume(const Mixer& mixer);

private:
	sf::Music _music;
	std::filesystem::path _path;
	float _volume;
	// Fade level from 0 (silent) to 1 (full), multiplied with the volume.
	float _gain;
	float _targetGain;
	// Gain change per second; 0 means no fade is running.
	float _fadeSpeed;
};

}
