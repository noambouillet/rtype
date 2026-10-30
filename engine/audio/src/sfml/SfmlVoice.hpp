/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** One reusable SFML sound slot
*/

#pragma once

#include <SFML/Audio.hpp>

#include "engine/audio/Bus.hpp"
#include "engine/audio/Mixer.hpp"

namespace engine::audio {

class SfmlVoice {
public:
	explicit SfmlVoice(const sf::SoundBuffer& buffer);

	[[nodiscard]] bool isFinished() const;
	void setBuffer(const sf::SoundBuffer& buffer);
	void setPan(float pan);
	void play(Bus bus, float volume, const Mixer& mixer);
	void applyVolume(const Mixer& mixer);

private:
	sf::Sound _sound;
	Bus _bus;
	float _volume;
};

}
