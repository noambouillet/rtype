/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Bus volumes and final sound volume computation
*/

#pragma once

#include <array>

#include "engine/audio/Bus.hpp"

namespace engine::audio {

class Mixer {
public:
	static constexpr float MIN_VOLUME = 0.0F;
	static constexpr float MAX_VOLUME = 100.0F;

	Mixer();

	static float clampVolume(float volume);
	static float stereoPan(float x, float listenerX, float range);

	void setVolume(Bus bus, float volume);
	[[nodiscard]] float volume(Bus bus) const;
	[[nodiscard]] float mix(Bus bus, float soundVolume) const;

private:
	std::array<float, BUS_COUNT> _volumes;
};

}
