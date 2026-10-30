/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio system
*/

#pragma once

#include <string>

#include "engine/audio/AudioBackend.hpp"
#include "engine/core/ecs/Coordinator.hpp"
#include "engine/core/ecs/System.hpp"

namespace engine::audio {

class AudioSystem : public core::System {
public:
	AudioSystem();
	static core::Signature signature(core::Coordinator& coordinator);
	void setMusic(std::string name, float volume, Seconds fade);
	void play(core::Coordinator& coordinator, IAudioBackend& backend, float deltaTime);

private:
	std::string _music;
	float _musicVolume;
	Seconds _musicFade;
	bool _musicStarted;
};

}
