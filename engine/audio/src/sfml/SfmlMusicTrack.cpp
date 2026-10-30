/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** One SFML music stream that can fade in and out
*/

#include "SfmlMusicTrack.hpp"

#include <algorithm>

namespace engine::audio {

SfmlMusicTrack::SfmlMusicTrack() : _volume(0.0F), _gain(0.0F), _targetGain(0.0F), _fadeSpeed(0.0F)
{
}

bool SfmlMusicTrack::isPlaying() const
{
	return _music.getStatus() != sf::SoundSource::Status::Stopped;
}

/// Tells if this track is playing the file at path.
bool SfmlMusicTrack::isPlaying(const std::filesystem::path& path) const
{
	return isPlaying() && _path == path;
}

/// Opens a looping music file. Returns false, and keeps nothing, if it cannot be opened.
bool SfmlMusicTrack::open(const std::filesystem::path& path)
{
	if (!_music.openFromFile(path)) {
		return false;
	}
	_path = path;
	_music.setLooping(true);
	return true;
}

/// Starts the opened music from silence, fading in over fade.
void SfmlMusicTrack::start(float volume, Seconds fade, const Mixer& mixer)
{
	_volume = Mixer::clampVolume(volume);
	_gain = 0.0F;
	fadeTo(1.0F, fade);
	applyVolume(mixer);
	_music.play();
}

void SfmlMusicTrack::setVolume(float volume, const Mixer& mixer)
{
	_volume = Mixer::clampVolume(volume);
	applyVolume(mixer);
}

/// Moves the fade gain to target over fade, or at once if fade is zero. A zero gain stops the music.
void SfmlMusicTrack::fadeTo(float target, Seconds fade)
{
	_targetGain = target;
	if (fade > Seconds::zero()) {
		_fadeSpeed = 1.0F / fade.count();
		return;
	}
	_gain = target;
	_fadeSpeed = 0.0F;
	if (_gain <= 0.0F) {
		_music.stop();
	}
}

/// Advances the fade by deltaTime seconds and stops the music once it faded out.
void SfmlMusicTrack::update(float deltaTime, const Mixer& mixer)
{
	if (!isPlaying() || _fadeSpeed <= 0.0F) {
		return;
	}
	const float step = _fadeSpeed * deltaTime;
	_gain = _gain < _targetGain ? std::min(_gain + step, _targetGain) : std::max(_gain - step, _targetGain);
	if (_gain == _targetGain) {
		_fadeSpeed = 0.0F;
	}
	if (_gain <= 0.0F) {
		_music.stop();
	}
	applyVolume(mixer);
}

void SfmlMusicTrack::applyVolume(const Mixer& mixer)
{
	_music.setVolume(mixer.mix(Bus::Music, _volume) * _gain);
}

}
