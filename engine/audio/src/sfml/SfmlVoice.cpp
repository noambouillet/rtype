/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** One reusable SFML sound slot
*/

#include "SfmlVoice.hpp"

#include <algorithm>

namespace engine::audio {

SfmlVoice::SfmlVoice(const sf::SoundBuffer& buffer) : _sound(buffer), _bus(Bus::Sfx), _volume(0.0F)
{
}

bool SfmlVoice::isFinished() const
{
	return _sound.getStatus() == sf::SoundSource::Status::Stopped;
}

/// Stops the current sound and loads another one into this slot.
void SfmlVoice::setBuffer(const sf::SoundBuffer& buffer)
{
	_sound.stop();
	_sound.setBuffer(buffer);
}

/// Sets the pan from -1 (left) to 1 (right). It only moves mono sounds.
void SfmlVoice::setPan(float pan)
{
	_sound.setPan(std::clamp(pan, -1.0F, 1.0F));
}

/// Plays the sound once on bus.
void SfmlVoice::play(Bus bus, float volume, const Mixer& mixer)
{
	_bus = bus;
	_volume = Mixer::clampVolume(volume);
	applyVolume(mixer);
	_sound.play();
}

void SfmlVoice::applyVolume(const Mixer& mixer)
{
	_sound.setVolume(mixer.mix(_bus, _volume));
}

}
