/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Bus volumes stored in the settings file
*/

#include "engine/audio/AudioSettings.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <optional>
#include <string>
#include <system_error>

#include "engine/audio/Mixer.hpp"

namespace engine::audio {

/// Parses the whole string as a float, or returns nothing.
static std::optional<float> parseVolume(const std::string& text)
{
	float value = 0.0F;
	const char* end = text.data() + text.size();
	const auto [stop, error] = std::from_chars(text.data(), end, value);
	if (error != std::errc{} || stop != end) {
		return std::nullopt;
	}
	return value;
}

/// Formats a volume without trailing zeros ("42.5", not "42.500000").
static std::string formatVolume(float volume)
{
	std::array<char, 32> buffer{};
	const auto [stop, error] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), volume);
	if (error != std::errc{}) {
		return "0";
	}
	return {buffer.data(), stop};
}

AudioSettings::AudioSettings() : _volumes()
{
}

/// Reads the volume_* keys. Missing or invalid values keep the current volume.
void AudioSettings::loadFromMap(const std::map<std::string, std::string>& data)
{
	for (std::size_t bus = 0; bus < BUS_COUNT; bus++) {
		const auto entry = data.find(std::string(KEYS[bus]));
		if (entry == data.end()) {
			continue;
		}
		if (const std::optional<float> value = parseVolume(entry->second)) {
			_volumes.setVolume(static_cast<Bus>(bus), *value);
		}
	}
}

void AudioSettings::saveToMap(std::map<std::string, std::string>& data) const
{
	for (std::size_t bus = 0; bus < BUS_COUNT; bus++) {
		data[std::string(KEYS[bus])] = formatVolume(_volumes.volume(static_cast<Bus>(bus)));
	}
}

void AudioSettings::apply(IAudioBackend& backend) const
{
	for (std::size_t bus = 0; bus < BUS_COUNT; bus++) {
		backend.setBusVolume(static_cast<Bus>(bus), _volumes.volume(static_cast<Bus>(bus)));
	}
}

void AudioSettings::setVolume(Bus bus, float volume)
{
	_volumes.setVolume(bus, volume);
}

float AudioSettings::volume(Bus bus) const
{
	return _volumes.volume(bus);
}

}
