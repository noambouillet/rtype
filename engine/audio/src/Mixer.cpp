/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Bus volumes and final sound volume computation
*/

#include "engine/audio/Mixer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace engine::audio {

/// Limits a volume to 0-100.
float Mixer::clampVolume(float volume)
{
	if (std::isnan(volume)) {
		return MIN_VOLUME;
	}
	return std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
}

/// Returns a pan from -1 (left) to 1 (right) for a sound at x heard from listenerX.
float Mixer::stereoPan(float x, float listenerX, float range)
{
	if (range <= 0.0F) {
		return 0.0F;
	}
	return std::clamp((x - listenerX) / range, -1.0F, 1.0F);
}

Mixer::Mixer() : _volumes{}
{
	_volumes.fill(MAX_VOLUME);
}

void Mixer::setVolume(Bus bus, float volume)
{
	_volumes[static_cast<std::size_t>(bus)] = clampVolume(volume);
}

float Mixer::volume(Bus bus) const
{
	return _volumes[static_cast<std::size_t>(bus)];
}

/// Returns the final 0-100 volume of a sound: sound x bus x master.
float Mixer::mix(Bus bus, float soundVolume) const
{
	const float busGain = volume(bus) / MAX_VOLUME;
	const float masterGain = bus == Bus::Master ? 1.0F : volume(Bus::Master) / MAX_VOLUME;
	return clampVolume(soundVolume) * busGain * masterGain;
}

}
