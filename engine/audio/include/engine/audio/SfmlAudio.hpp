/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML audio backend
*/

#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>

#include "engine/audio/AudioBackend.hpp"

namespace engine::audio {

class SfmlAudio : public IAudioBackend {
public:
	// Sounds playing at once. Past this, the oldest sound is cut.
	static constexpr std::size_t MAX_VOICES = 32;

	explicit SfmlAudio(std::filesystem::path root = {});
	~SfmlAudio() override;
	SfmlAudio(const SfmlAudio&) = delete;
	SfmlAudio& operator=(const SfmlAudio&) = delete;

	void playMusic(std::string_view name, float volume, Seconds fade) override;
	void stopMusic(Seconds fade) override;
	void playSound(std::string_view name, float volume, Bus bus, float pan) override;
	void setBusVolume(Bus bus, float volume) override;
	[[nodiscard]] float busVolume(Bus bus) const override;
	void update(float deltaTime) override;

private:
	// Holds the SFML objects, so code including this header does not depend on SFML.
	class Implementation;
	std::unique_ptr<Implementation> _implementation;
};

}
