/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** SFML audio backend
*/

#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include "engine/audio/AudioBackend.hpp"

namespace engine::audio {

class SfmlAudio : public IAudioBackend {
public:
	explicit SfmlAudio(std::filesystem::path root);
	~SfmlAudio() override;
	SfmlAudio(const SfmlAudio&) = delete;
	SfmlAudio& operator=(const SfmlAudio&) = delete;

	void playMusic(std::string_view name, float volume) override;
	void stopMusic() override;
	void playSound(std::string_view name, float volume) override;

private:
	// SFML types stay in the .cpp, so games never include SFML headers.
	struct Implementation;
	std::unique_ptr<Implementation> _implementation;
};

}
