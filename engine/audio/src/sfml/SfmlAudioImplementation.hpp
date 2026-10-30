/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML objects owned by SfmlAudio, kept out of the public header
*/

#pragma once

#include <SFML/Audio.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "engine/audio/Bus.hpp"
#include "engine/audio/Mixer.hpp"
#include "engine/audio/SfmlAudio.hpp"

namespace engine::audio {

/// One sound slot, kept after the sound ends so the next sound can reuse it.
struct SfmlVoice {
	SfmlVoice(const sf::SoundBuffer& buffer, Bus bus, float volume);

	sf::Sound sound;
	Bus bus;
	float volume;
};

/// One music stream. Two of them allow a crossfade.
struct SfmlMusicTrack {
	SfmlMusicTrack();

	[[nodiscard]] bool isPlaying() const;
	void fadeTo(float target, Seconds fade);

	sf::Music music;
	std::filesystem::path path;
	float volume;
	// Fade gain from 0 to 1, multiplied with the volume.
	float gain;
	float targetGain;
	// Gain change per second; 0 means no fade in progress.
	float fadeSpeed;
};

struct SfmlAudio::Implementation {
	static constexpr std::size_t TRACK_COUNT = 2;

	Implementation();

	[[nodiscard]] std::filesystem::path resolve(std::string_view name) const;
	sf::SoundBuffer& loadBuffer(const std::string& name);
	SfmlVoice& acquireVoice(const sf::SoundBuffer& buffer, Bus bus, float volume);
	void applyVolume(SfmlVoice& voice) const;
	void applyVolume(SfmlMusicTrack& track) const;
	SfmlMusicTrack& currentTrack();
	SfmlMusicTrack& otherTrack();

	std::filesystem::path root;
	Mixer mixer;
	std::array<SfmlMusicTrack, TRACK_COUNT> tracks;
	std::size_t current;
	std::unordered_map<std::string, sf::SoundBuffer> buffers;
	std::unordered_set<std::string> missing;
	// Ordered by start time: the front voice is the oldest.
	std::list<SfmlVoice> voices;
};

}
