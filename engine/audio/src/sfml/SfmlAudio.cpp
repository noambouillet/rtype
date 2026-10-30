/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML audio backend
*/

#include "engine/audio/SfmlAudio.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "SfmlAudioImplementation.hpp"
#include "engine/audio/AudioError.hpp"

namespace engine::audio {

SfmlVoice::SfmlVoice(const sf::SoundBuffer& buffer, Bus bus, float volume) : sound(buffer), bus(bus), volume(volume)
{
}

SfmlMusicTrack::SfmlMusicTrack() : volume(0.0F), gain(0.0F), targetGain(0.0F), fadeSpeed(0.0F)
{
}

bool SfmlMusicTrack::isPlaying() const
{
	return music.getStatus() != sf::SoundSource::Status::Stopped;
}

/// Moves the fade gain to target over fade, or at once if fade is zero.
void SfmlMusicTrack::fadeTo(float target, Seconds fade)
{
	targetGain = target;
	if (fade <= Seconds::zero()) {
		gain = target;
		fadeSpeed = 0.0F;
		return;
	}
	fadeSpeed = 1.0F / fade.count();
}

SfmlAudio::Implementation::Implementation() : current(0)
{
}

std::filesystem::path SfmlAudio::Implementation::resolve(std::string_view name) const
{
	if (root.empty()) {
		return {name};
	}
	return root / std::filesystem::path(name).lexically_normal();
}

/// Loads a sound into the cache, or throws AudioError if the file cannot be read.
sf::SoundBuffer& SfmlAudio::Implementation::loadBuffer(const std::string& name)
{
	const auto [bufferEntry, wasInserted] = buffers.try_emplace(name);
	if (wasInserted) {
		const std::filesystem::path soundPath = resolve(name);
		if (!bufferEntry->second.loadFromFile(soundPath)) {
			buffers.erase(bufferEntry);
			throw AudioError("Cannot load sound: " + soundPath.string());
		}
	}
	return bufferEntry->second;
}

/// Returns a voice for buffer: a finished one, else a new one, else the oldest one cut.
SfmlVoice& SfmlAudio::Implementation::acquireVoice(const sf::SoundBuffer& buffer, Bus bus, float volume)
{
	auto voice = std::ranges::find_if(voices, [](const SfmlVoice& candidate) { return candidate.sound.getStatus() == sf::SoundSource::Status::Stopped; });
	if (voice == voices.end()) {
		if (voices.size() < MAX_VOICES) {
			return voices.emplace_back(buffer, bus, volume);
		}
		voice = voices.begin();
		voice->sound.stop();
	}
	voice->sound.setBuffer(buffer);
	voice->bus = bus;
	voice->volume = volume;
	voices.splice(voices.end(), voices, voice);
	return voices.back();
}

void SfmlAudio::Implementation::applyVolume(SfmlVoice& voice) const
{
	voice.sound.setVolume(mixer.mix(voice.bus, voice.volume));
}

void SfmlAudio::Implementation::applyVolume(SfmlMusicTrack& track) const
{
	track.music.setVolume(mixer.mix(Bus::Music, track.volume) * track.gain);
}

SfmlMusicTrack& SfmlAudio::Implementation::currentTrack()
{
	return tracks[current];
}

SfmlMusicTrack& SfmlAudio::Implementation::otherTrack()
{
	return tracks[(current + 1) % TRACK_COUNT];
}

/// Sound and music names are file paths relative to root, or direct paths if root is empty.
SfmlAudio::SfmlAudio(std::filesystem::path root) : _implementation(std::make_unique<Implementation>())
{
	_implementation->root = std::move(root);
}

SfmlAudio::~SfmlAudio() = default;

/// Plays looping music, crossfading from the previous one over fade.
/// The music already playing only gets its volume changed.
void SfmlAudio::playMusic(std::string_view name, float volume, Seconds fade)
{
	const std::filesystem::path path = _implementation->resolve(name);
	SfmlMusicTrack& previous = _implementation->currentTrack();
	if (previous.isPlaying() && previous.path == path) {
		previous.volume = Mixer::clampVolume(volume);
		previous.fadeTo(1.0F, fade);
		_implementation->applyVolume(previous);
		return;
	}
	SfmlMusicTrack& next = _implementation->otherTrack();
	if (!next.music.openFromFile(path)) {
		return;
	}
	if (previous.isPlaying()) {
		previous.fadeTo(0.0F, fade);
		if (fade <= Seconds::zero()) {
			previous.music.stop();
		}
	}
	next.path = path;
	next.volume = Mixer::clampVolume(volume);
	next.gain = fade > Seconds::zero() ? 0.0F : 1.0F;
	next.fadeTo(1.0F, fade);
	next.music.setLooping(true);
	_implementation->applyVolume(next);
	next.music.play();
	_implementation->current = (_implementation->current + 1) % Implementation::TRACK_COUNT;
}

/// Stops the music, fading it out over fade.
void SfmlAudio::stopMusic(Seconds fade)
{
	for (SfmlMusicTrack& track : _implementation->tracks) {
		if (fade <= Seconds::zero()) {
			track.music.stop();
		}
		track.fadeTo(0.0F, fade);
	}
}

/// Plays a sound once on bus, or nothing if its file is missing.
/// Pan goes from -1 (left) to 1 (right) and only moves mono sounds.
void SfmlAudio::playSound(std::string_view name, float volume, Bus bus, float pan)
{
	const std::string key(name);
	if (_implementation->missing.contains(key)) {
		return;
	}
	try {
		const sf::SoundBuffer& buffer = _implementation->loadBuffer(key);
		SfmlVoice& voice = _implementation->acquireVoice(buffer, bus, Mixer::clampVolume(volume));
		voice.sound.setPan(std::clamp(pan, -1.0F, 1.0F));
		_implementation->applyVolume(voice);
		voice.sound.play();
	} catch (const AudioError&) {
		_implementation->missing.insert(key);
	}
}

/// Sets a bus volume. Sounds and music already playing change at once.
void SfmlAudio::setBusVolume(Bus bus, float volume)
{
	_implementation->mixer.setVolume(bus, volume);
	for (SfmlVoice& voice : _implementation->voices) {
		_implementation->applyVolume(voice);
	}
	for (SfmlMusicTrack& track : _implementation->tracks) {
		_implementation->applyVolume(track);
	}
}

float SfmlAudio::busVolume(Bus bus) const
{
	return _implementation->mixer.volume(bus);
}

/// Advances music fades. Call once per frame.
void SfmlAudio::update(float deltaTime)
{
	for (SfmlMusicTrack& track : _implementation->tracks) {
		if (!track.isPlaying() || track.fadeSpeed <= 0.0F) {
			continue;
		}
		const float step = track.fadeSpeed * deltaTime;
		if (track.gain < track.targetGain) {
			track.gain = std::min(track.gain + step, track.targetGain);
		} else {
			track.gain = std::max(track.gain - step, track.targetGain);
		}
		if (track.gain == track.targetGain) {
			track.fadeSpeed = 0.0F;
		}
		if (track.gain <= 0.0F) {
			track.music.stop();
		}
		_implementation->applyVolume(track);
	}
}

}
