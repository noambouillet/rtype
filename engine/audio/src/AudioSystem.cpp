/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Audio system
*/

#include "engine/audio/AudioSystem.hpp"

#include <utility>
#include <vector>

#include "engine/audio/Components.hpp"

namespace engine::audio {

AudioSystem::AudioSystem() : _music{}, _musicVolume(0.0F), _musicStarted(false)
{
}

/// Configures an optional looping music for this world, started on the next play.
void AudioSystem::setMusic(std::string name, float volume)
{
	_music = std::move(name);
	_musicVolume = volume;
	_musicStarted = false;
}

core::Signature AudioSystem::signature(core::Coordinator& coordinator)
{
	core::Signature signature;
	signature.set(coordinator.getComponentType<Sound>());
	return signature;
}

/// Plays every pending one-shot sound entity once, then destroys it.
void AudioSystem::play(core::Coordinator& coordinator, IAudioBackend& backend)
{
	if (!_musicStarted && !_music.empty()) {
		backend.playMusic(_music, _musicVolume);
		_musicStarted = true;
	}
	const std::vector<core::Entity> played(entities.begin(), entities.end());
	for (core::Entity entity : played) {
		const Sound& sound = coordinator.getComponent<Sound>(entity);
		backend.playSound(sound.name, sound.volume);
		coordinator.destroyEntity(entity);
	}
}

}
