/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Audio system
*/

#pragma once

#include <string>

#include "Coordinator.hpp"
#include "System.hpp"
#include "game_engine/Audio/AudioBackend.hpp"

namespace rtype::ecs {

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
