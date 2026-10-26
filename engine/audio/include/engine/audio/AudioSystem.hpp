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

namespace engine::core {

class AudioSystem : public System {
public:
	AudioSystem();
	Signature signature(Coordinator& coordinator);
	void setMusic(std::string name, float volume);
	void play(Coordinator& coordinator, audio::IAudioBackend& backend);

private:
	std::string _music;
	float _musicVolume;
	bool _musicStarted;
};

}
