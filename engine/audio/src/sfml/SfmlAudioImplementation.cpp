/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML objects owned by SfmlAudio, kept out of the public header
*/

#include "SfmlAudioImplementation.hpp"

#include <algorithm>
#include <utility>

#include "engine/audio/AudioError.hpp"

namespace engine::audio {

/// Sound and music names are file paths relative to root, or direct paths if root is empty.
SfmlAudio::Implementation::Implementation(std::filesystem::path root) : _root(std::move(root)), _mixer(), _tracks(), _current(0)
{
}

/// Plays looping music, crossfading from the previous one over fade.
/// The music already playing only gets its volume changed.
void SfmlAudio::Implementation::playMusic(std::string_view name, float volume, Seconds fade)
{
	const std::filesystem::path path = resolve(name);
	SfmlMusicTrack& previous = currentTrack();
	if (previous.isPlaying(path)) {
		previous.setVolume(volume, _mixer);
		previous.fadeTo(1.0F, fade);
		return;
	}
	SfmlMusicTrack& next = otherTrack();
	if (!next.open(path)) {
		return;
	}
	if (previous.isPlaying()) {
		previous.fadeTo(0.0F, fade);
	}
	next.start(volume, fade, _mixer);
	_current = (_current + 1) % TRACK_COUNT;
}

/// Stops the music, fading it out over fade.
void SfmlAudio::Implementation::stopMusic(Seconds fade)
{
	for (SfmlMusicTrack& track : _tracks) {
		track.fadeTo(0.0F, fade);
	}
}

/// Plays a sound once on bus, or nothing if its file is missing.
void SfmlAudio::Implementation::playSound(std::string_view name, float volume, Bus bus, float pan)
{
	const std::string key(name);
	if (_missing.contains(key)) {
		return;
	}
	try {
		SfmlVoice& voice = acquireVoice(loadBuffer(key));
		voice.setPan(pan);
		voice.play(bus, volume, _mixer);
	} catch (const AudioError&) {
		_missing.insert(key);
	}
}

/// Sets a bus volume. Sounds and music already playing change at once.
void SfmlAudio::Implementation::setBusVolume(Bus bus, float volume)
{
	_mixer.setVolume(bus, volume);
	for (SfmlVoice& voice : _voices) {
		voice.applyVolume(_mixer);
	}
	for (SfmlMusicTrack& track : _tracks) {
		track.applyVolume(_mixer);
	}
}

float SfmlAudio::Implementation::busVolume(Bus bus) const
{
	return _mixer.volume(bus);
}

void SfmlAudio::Implementation::update(float deltaTime)
{
	for (SfmlMusicTrack& track : _tracks) {
		track.update(deltaTime, _mixer);
	}
}

std::filesystem::path SfmlAudio::Implementation::resolve(std::string_view name) const
{
	if (_root.empty()) {
		return {name};
	}
	return _root / std::filesystem::path(name).lexically_normal();
}

/// Loads a sound into the cache, or throws AudioError if the file cannot be read.
sf::SoundBuffer& SfmlAudio::Implementation::loadBuffer(const std::string& name)
{
	const auto [bufferEntry, wasInserted] = _buffers.try_emplace(name);
	if (wasInserted) {
		const std::filesystem::path soundPath = resolve(name);
		if (!bufferEntry->second.loadFromFile(soundPath)) {
			_buffers.erase(bufferEntry);
			throw AudioError("Cannot load sound: " + soundPath.string());
		}
	}
	return bufferEntry->second;
}

/// Returns a voice for buffer: a finished one, else a new one, else the oldest one cut.
SfmlVoice& SfmlAudio::Implementation::acquireVoice(const sf::SoundBuffer& buffer)
{
	auto voice = std::ranges::find_if(_voices, [](const SfmlVoice& candidate) { return candidate.isFinished(); });
	if (voice == _voices.end()) {
		if (_voices.size() < MAX_VOICES) {
			return _voices.emplace_back(buffer);
		}
		voice = _voices.begin();
	}
	voice->setBuffer(buffer);
	_voices.splice(_voices.end(), _voices, voice);
	return _voices.back();
}

SfmlMusicTrack& SfmlAudio::Implementation::currentTrack()
{
	return _tracks[_current];
}

SfmlMusicTrack& SfmlAudio::Implementation::otherTrack()
{
	return _tracks[(_current + 1) % TRACK_COUNT];
}

}
