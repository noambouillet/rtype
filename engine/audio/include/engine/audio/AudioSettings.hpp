/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Bus volumes stored in the settings file
*/

#pragma once

#include <array>
#include <map>
#include <string>
#include <string_view>

#include "engine/audio/AudioBackend.hpp"
#include "engine/audio/Bus.hpp"
#include "engine/audio/Mixer.hpp"

namespace engine::audio {

class AudioSettings {
public:
	static constexpr std::array<std::string_view, BUS_COUNT> KEYS = {"volume_master", "volume_music", "volume_sfx", "volume_ui"};

	AudioSettings();

	void loadFromMap(const std::map<std::string, std::string>& data);
	void saveToMap(std::map<std::string, std::string>& data) const;
	void apply(IAudioBackend& backend) const;
	void setVolume(Bus bus, float volume);
	[[nodiscard]] float volume(Bus bus) const;

private:
	Mixer _volumes;
};

}
