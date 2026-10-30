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

#include "SfmlMusicTrack.hpp"
#include "SfmlVoice.hpp"
#include "engine/audio/Bus.hpp"
#include "engine/audio/Mixer.hpp"
#include "engine/audio/SfmlAudio.hpp"

namespace engine::audio {

class SfmlAudio::Implementation {
public:
	// Two music tracks let the old music fade out while the new one fades in.
	static constexpr std::size_t TRACK_COUNT = 2;

	explicit Implementation(std::filesystem::path root);

	void playMusic(std::string_view name, float volume, Seconds fade);
	void stopMusic(Seconds fade);
	void playSound(std::string_view name, float volume, Bus bus, float pan);
	void setBusVolume(Bus bus, float volume);
	[[nodiscard]] float busVolume(Bus bus) const;
	void update(float deltaTime);

private:
	[[nodiscard]] std::filesystem::path resolve(std::string_view name) const;
	sf::SoundBuffer& loadBuffer(const std::string& name);
	SfmlVoice& acquireVoice(const sf::SoundBuffer& buffer);
	SfmlMusicTrack& currentTrack();
	SfmlMusicTrack& otherTrack();

	std::filesystem::path _root;
	Mixer _mixer;
	std::array<SfmlMusicTrack, TRACK_COUNT> _tracks;
	std::size_t _current;
	std::unordered_map<std::string, sf::SoundBuffer> _buffers;
	std::unordered_set<std::string> _missing;
	// Ordered by start time: the front voice is the oldest.
	std::list<SfmlVoice> _voices;
};

}
